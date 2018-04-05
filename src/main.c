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

int main(int argn, char **argv)
{
	unsigned int row;
	struct pam inpam, outpam;

	tuple **in_img, **out_img;

	pm_init(argv[0], 0);
	in_img = pnm_readpam(stdin, &inpam, sizeof(inpam));
	outpam = inpam; outpam.file = stdout;
	out_img = pnm_allocpamarray(&outpam);
	sobel(in_img, out_img, 0, 0,
		inpam.width, inpam.height);
	pnm_freepamarray(in_img, &inpam);
	pnm_writepam(&outpam, out_img);
	pnm_freepamarray(out_img, &outpam);
	return 0;
}