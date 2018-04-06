#include "precompiled.h"

tuple **pnm_allocpamstripe(struct pam info, int rows)
{
	info.height = rows;
	return pnm_allocpamarray(&info);
}

void pnm_readpamstripe(struct pam *info, tuple **stripe, int rows)
{
	for (int i = 0; i < rows; i++)
		pnm_readpamrow(info, stripe[i]);
}

void pnm_writepamstripe(struct pam *info, tuple **stripe, int rows)
{
	for (int i = 0; i < rows; i++)
		pnm_writepamrow(info, stripe[i]);
}

void pnm_freepamstripe(struct pam info, tuple **stripe, int rows)
{
	info.height = rows;
	pnm_freepamarray(stripe, &info);
}



