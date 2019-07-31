#ifndef MD5_H
#define MD5_H 

typedef struct
{
    unsigned int state[4];             /* state (ABCD) */
    unsigned int count[2];             /* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64];        /* input buffer */
} MD5_CTX;

void MD5Init(MD5_CTX *context);

void MD5Update(MD5_CTX *context, unsigned char *, unsigned int);

void MD5Final(unsigned char[16], MD5_CTX *context);

int MD5String(char *in, int in_len, unsigned char *out);

int MD5toStr(unsigned char *src,int len,char *out);
#endif
