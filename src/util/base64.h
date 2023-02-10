#ifndef BASE64_H_
#define BASE64_H_

#define BASE64_ENCODE_OUT_SIZE(s) (((s) + 2) / 3 * 4)
#define BASE64_DECODE_OUT_SIZE(s) (((s)) / 4 * 3)

// @return encoded size
int base64_encode(const unsigned char *in, unsigned int inlen, char *out);

// @return decoded size
int base64_decode(const char *in, unsigned int inlen, unsigned char *out);

#endif  // BASE64_H_
