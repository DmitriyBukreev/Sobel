tuple **pnm_allocpamstripe(struct pam info, int rows);

void pnm_readpamstripe(struct pam *info, tuple **stripe, int rows);

void pnm_writepamstripe(struct pam *info, tuple **stripe, int rows);

void pnm_freepamstripe(struct pam info, tuple **stripe, int rows);