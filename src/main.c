#include "precompiled.h"

struct arg_struct {
	tuple **src;
	tuple **dst;
	int x0;
	int y0;
	int xf;
	int yf;
};

#define KSIZE 3
void sobel(tuple **src, tuple **dst, int x0, int y0,
	int xf, int yf)
{
	int x, y, i, j;
	// Take the other kernel?
	double kernel[KSIZE][KSIZE] = {
		{-1, 0, 1},
		{-2, 0, 2},
		{-1, 0, 1}
	};
	double sum;

	for (y = y0 + 1; y < yf - 1; y++)
		for (x = x0 + 1; x < xf - 1; x++) {
			sum = 0;
			for (i = 0; i < KSIZE; i++)
				for (j = 0; j < KSIZE; j++)
					sum +=
					src[y+i-1][x+j-1][0] * kernel[i][j];
			dst[y][x][0] = sum / 9;
		}
}

void *sobel_thread(void *in_args)
{
	struct arg_struct *args = in_args;

	sobel(args->src, args->dst, args->x0, args->y0,
		args->xf, args->yf);
	pthread_exit(NULL);
}

// 250 MB is enough ;)
#define MAXMEM 250000000
int main(int argn, char **argv)
{
	// Size of one row
	unsigned long int sizeofrow;
	// Number of rows to allocate in memory
	int rownum, savedrownum;
	// Information for pam functions
	struct pam inpam, outpam;
	// Arguments for threads
	struct arg_struct *args;
	// Thread ID array
	pthread_t *threads;
	// Error code from pthread
	int result;
	int threadnum = 1;
	int sizeofsubstripe;
	// Variables for time estimation
	time_t st_time, end_time;
	int res_time;
	// Variables for argv processing
	// Flag validator of input
	char f_wrong;
	// stdin, file, etc.
	FILE *in_stream;
	// Result from getopt
	int opt;
	// Number of options provided
	char has_option;
	// Fifo ID
	int fifo;
	// Arrays for images in RAM
	tuple **in_img, **out_img;

	// Processing arguments
	in_stream = stdin;
	has_option = f_wrong = 0;
	while ((opt = getopt(argn, argv, "j:")) != -1) {
		switch (opt) {
		case 'j':
			threadnum = atoi(optarg);
			has_option += 2;
			if (threadnum < 1) {
				fprintf(stderr, "Wrong number of threads\n");
				exit(EXIT_FAILURE);
			}
			break;
		case '?':
			f_wrong = 1;
		}
	}
	if (f_wrong != 0) {
		fprintf(stderr, "Wrong argumens\n");
		exit(EXIT_FAILURE);
	}
	if (argn > (has_option + 1)) {
		in_stream = fopen(argv[has_option + 1], "rb");
		HANDLE_ERROR(in_stream, NULL);
	}

	// Initialization of libnetpbm
	pm_init(argv[0], 0);
	// Reading header and saving it in the file
	pnm_readpaminit(in_stream, &inpam, sizeof(inpam));
	outpam = inpam; outpam.file = stdout;
	pnm_writepaminit(&outpam);
	// Calculating number of rows to fit
	// MAXMEM limitation
	sizeofrow = inpam.width*inpam.depth*sizeof(sample);
	rownum = ceil(MAXMEM/sizeofrow);

	// Allocating memory for input and output stripes
	in_img = pnm_allocpamstripe(inpam, rownum);
	out_img = pnm_allocpamstripe(outpam, rownum);
	// Saving rownum to free memory further
	savedrownum = rownum;
	// Processing image stripe by stripe
	args = malloc(sizeof(struct arg_struct) * threadnum);
	threads = malloc(sizeof(pthread_t) * threadnum);

	// Separating stripe into substripes for each thread
	sizeofsubstripe = ceil((double)rownum/threadnum);
	for (int i = 0, j = 0; i < rownum; i += sizeofsubstripe, j++) {
		args[j].src = in_img;
		args[j].dst = out_img;
		args[j].x0 = 0;
		args[j].xf = inpam.width;
		args[j].yf = i + sizeofsubstripe;
		args[j].y0 = i;
		if (args[j].yf > rownum)
			args[j].yf = i + (rownum - args[j].y0);
		if (j != 0)
			args[j].y0 -= 2;
	}
	res_time = 0;
	for (int y = 0; y < inpam.height; y += rownum) {
		if (rownum > (inpam.height - y)) {
			sizeofsubstripe = ceil((double)rownum/threadnum);
			// Last stripe may be of a different height
			rownum = inpam.height - y;
			// Recalculating substripes
			for (int i = 0, j = 0; i < rownum;
				i += sizeofsubstripe, j++) {
				args[j].y0 = i;
				args[j].yf = i + sizeofsubstripe;
				if (args[j].yf > rownum)
					args[j].yf = i + (rownum - args[j].y0);
				if (j != 0)
					args[j].y0 -= 2;
			}
		}
		pnm_readpamstripe(&inpam, in_img, rownum);
		// Start of processing
		st_time = time(NULL);
		for (int i = 1; i < threadnum; i++) {
			result = pthread_create(&threads[i], NULL,
			sobel_thread, &args[i]);
			if (result != 0) {
				perror("Thread creation failed\n");
				exit(EXIT_FAILURE);
			}
		}
		sobel(in_img, out_img, 0, 0, args[0].xf, args[0].yf);
		for (int i = 1; i < threadnum; i++) {
			result = pthread_join(threads[i], NULL);
			if (result != 0) {
				perror("Thread join failed\n");
				exit(EXIT_FAILURE);
			}
		}
		// End of processing
		end_time = time(NULL);
		res_time += difftime(end_time, st_time);
		pnm_writepamstripe(&outpam, out_img, rownum);
	}
	pnm_freepamstripe(inpam, in_img, savedrownum);
	pnm_freepamstripe(inpam, out_img, savedrownum);
	free(args);
	free(threads);
	fprintf(stderr, "Time estimation: %i\n", res_time);
	// For stats collecting
	fifo = open("/tmp/stat_fifo", O_WRONLY);
	HANDLE_ERROR(fifo, -1);
	HANDLE_ERROR(write(fifo, &res_time, 4), -1);
	HANDLE_ERROR(close(fifo), -1);
	return 0;
}