#include "precompiled.h"

#define KSIZE 3
void sobel(tuple * *src, tuple * *dst, int x0, int y0,
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

// 0.5 GB is enough ;)
#define MAXMEM 500000000
int main(int argn, char **argv)
{
	unsigned long int sizeofrow;
	int rownum, savedrownum;
	struct pam inpam, outpam;

	tuple **in_img, **out_img;

	pm_init(argv[0], 0);
	pnm_readpaminit(stdin, &inpam, sizeof(inpam));
	outpam = inpam; outpam.file = stdout;
	pnm_writepaminit(&outpam);
	sizeofrow = inpam.width*inpam.depth*sizeof(sample);
	rownum = ceil(MAXMEM/sizeofrow);

	fprintf(stderr, "Memory provided = %i bytes\t", MAXMEM);
	fprintf(stderr, "Size of row = %lu bytes\n", sizeofrow);
	fprintf(stderr, "Image height= %i\t", inpam.height);
	fprintf(stderr, "Image separated into %f stripes\n",
		ceil(inpam.height*sizeofrow/MAXMEM));
	fprintf(stderr, "Each stripe has %i rows\n",
		rownum);

	in_img = pnm_allocpamstripe(inpam, rownum);
	out_img = pnm_allocpamstripe(outpam, rownum);
	savedrownum = rownum;
	for (int y = 0; y < inpam.height; y += rownum) {
		if (rownum > (inpam.height - y))
			rownum = inpam.height - y;
		fprintf(stderr, "%i more rows left\n", inpam.height - y);
		pnm_readpamstripe(&inpam, in_img, rownum);
		sobel(in_img, out_img, 0, 0, inpam.width, rownum);
		pnm_writepamstripe(&outpam, out_img, rownum);
	}
	pnm_freepamstripe(inpam, in_img, savedrownum);
	pnm_freepamstripe(inpam, out_img, savedrownum);

	return 0;
}