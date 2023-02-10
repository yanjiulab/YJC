#ifndef MD5_H_
#define MD5_H_

typedef struct {
    unsigned int count[2];
    unsigned int state[4];
    unsigned char buffer[64];
} MD5_CTX;

void MD5Init(MD5_CTX *ctx);
void MD5Update(MD5_CTX *ctx, unsigned char *input, unsigned int inputlen);
void MD5Final(MD5_CTX *ctx, unsigned char digest[16]);

void md5(unsigned char *input, unsigned int inputlen, unsigned char digest[16]);

// NOTE: if outputlen > 32: output[32] = '\0'
void md5_hex(unsigned char *input, unsigned int inputlen, char *output, unsigned int outputlen);

#endif  // MD5_H_
