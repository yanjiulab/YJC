#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"
#include "md5.h"
#include "sha1.h"
#include "test.h"
#include "sha256.h"

void test_util() {
    FILE *fp = fopen("/etc/passwd", "r");
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    char *in = malloc(len);
    fseek(fp, 0, SEEK_SET);
    fread(in, len, 1, fp);
    in[len] = '\0';
    fclose(fp);

    printf("len of /etc/passwd is %d\n", len);

    char encode_out[len * 2];
    bzero(encode_out, len * 2);
    int encode_len = base64_encode(in, len, encode_out);
    printf("base64 encoded: %s\n", encode_out);

    char decode_out[len];
    int decode_len = base64_decode(encode_out, encode_len, decode_out);
    printf("base64 decoded: %s\n", decode_out);

    char md5sum[33];
    md5_hex(in, len, md5sum, sizeof(md5sum));
    printf("md5sum: %s\n", md5sum);

    char sha1[41];
    sha1_hex(in, len, sha1, sizeof(sha1));
    printf("sha1: %s\n", sha1);

    char sha256[65];
    sha256_hex(in, len, sha256, sizeof(sha256));
    printf("sha256: %s\n", sha256);
}
