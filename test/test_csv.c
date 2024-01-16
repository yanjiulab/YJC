#include "csv.h"
#include "test.h"

#define CSVTEST 1

#ifdef CSVFIX
/*
csvfix - reads (possibly malformed) CSV data from input file
         and writes properly formed CSV to output file
*/

#include <csv.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cb1(void* s, size_t i, void* outfile) {
    csv_fwrite((FILE*)outfile, s, i);
    fputc(',', (FILE*)outfile);
}

void cb2(int c, void* outfile) {
    fseek((FILE*)outfile, -1, SEEK_CUR);
    fputc('\n', (FILE*)outfile);
}

int csvfix(int argc, char* argv[]) {
    char buf[1024];
    size_t i;
    struct csv_parser p;
    FILE *infile, *outfile;
    csv_init(&p, 0);

    if (argc != 3) {
        fprintf(stderr, "Usage: csv_fix infile outfile\n");
        return EXIT_FAILURE;
    }

    if (!strcmp(argv[1], argv[2])) {
        fprintf(stderr, "Input file and output file must not be the same!\n");
        exit(EXIT_FAILURE);
    }

    infile = fopen(argv[1], "rb");
    if (infile == NULL) {
        fprintf(stderr, "Failed to open file %s: %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    outfile = fopen(argv[2], "wb");
    if (outfile == NULL) {
        fprintf(stderr, "Failed to open file %s: %s\n", argv[2], strerror(errno));
        fclose(infile);
        exit(EXIT_FAILURE);
    }

    while ((i = fread(buf, 1, 1024, infile)) > 0) {
        if (csv_parse(&p, buf, i, cb1, cb2, outfile) != i) {
            fprintf(stderr, "Error parsing file: %s\n", csv_strerror(csv_error(&p)));
            fclose(infile);
            fclose(outfile);
            remove(argv[2]);
            exit(EXIT_FAILURE);
        }
    }

    csv_fini(&p, cb1, cb2, outfile);
    csv_free(&p);

    if (ferror(infile)) {
        fprintf(stderr, "Error reading from input file");
        fclose(infile);
        fclose(outfile);
        remove(argv[2]);
        exit(EXIT_FAILURE);
    }

    fclose(infile);
    fclose(outfile);
    return EXIT_SUCCESS;
}
#endif // CSVFIX

#ifdef CSVINFO
/*
csvinfo - reads CSV data from input file(s) and reports the number
          of fields and rows encountered in each file
*/

#include <csv.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct counts {
    long unsigned fields;
    long unsigned rows;
};

void cb1(void* s, size_t len, void* data) { ((struct counts*)data)->fields++; }
void cb2(int c, void* data) { ((struct counts*)data)->rows++; }

static int is_space(unsigned char c) {
    if (c == CSV_SPACE || c == CSV_TAB)
        return 1;
    return 0;
}

static int is_term(unsigned char c) {
    if (c == CSV_CR || c == CSV_LF)
        return 1;
    return 0;
}

int csvinfo(int argc, char* argv[]) {
    FILE* fp;
    struct csv_parser p;
    char buf[1024];
    size_t bytes_read;
    unsigned char options = 0;
    struct counts c = {0, 0};

    if (argc < 2) {
        fprintf(stderr, "Usage: csvinfo [-s] files\n");
        exit(EXIT_FAILURE);
    }

    if (csv_init(&p, options) != 0) {
        fprintf(stderr, "Failed to initialize csv parser\n");
        exit(EXIT_FAILURE);
    }

    csv_set_space_func(&p, is_space);
    csv_set_term_func(&p, is_term);

    while (*(++argv)) {
        if (strcmp(*argv, "-s") == 0) {
            options = CSV_STRICT;
            csv_set_opts(&p, options);
            continue;
        }

        fp = fopen(*argv, "rb");
        if (!fp) {
            fprintf(stderr, "Failed to open %s: %s\n", *argv, strerror(errno));
            continue;
        }

        while ((bytes_read = fread(buf, 1, 1024, fp)) > 0) {
            if (csv_parse(&p, buf, bytes_read, cb1, cb2, &c) != bytes_read) {
                fprintf(stderr, "Error while parsing file: %s\n", csv_strerror(csv_error(&p)));
            }
        }

        csv_fini(&p, cb1, cb2, &c);

        if (ferror(fp)) {
            fprintf(stderr, "Error while reading file %s\n", *argv);
            fclose(fp);
            continue;
        }

        fclose(fp);
        printf("%s: %lu fields, %lu rows\n", *argv, c.fields, c.rows);
    }

    csv_free(&p);
    exit(EXIT_SUCCESS);
}
#endif // CSVINFO

#ifdef CSVTEST
/*
csvtest - reads CSV data from stdin and output properly formed equivalent
          useful for testing the library
*/

#include <csv.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int put_comma;

void cb1(void* s, size_t i, void* p) {
    if (put_comma)
        putc(',', stdout);
    csv_fwrite(stdout, s, i);
    put_comma = 1;
}

void cb2(int c, void* p) {
    put_comma = 0;
    putc('\n', stdout);
}

int csvtest(void) {
    struct csv_parser p;
    int i;
    char c;

    csv_init(&p, 0);

    while ((i = getc(stdin)) != EOF) {
        c = i;
        if (csv_parse(&p, &c, 1, cb1, cb2, NULL) != 1) {
            fprintf(stderr, "Error: %s\n", csv_strerror(csv_error(&p)));
            exit(EXIT_FAILURE);
        }
    }

    csv_fini(&p, cb1, cb2, NULL);
    csv_free(&p);

    return EXIT_SUCCESS;
}

#endif // CSVTEST

#ifdef CSVVALID

/*
csvvalid - determine if files are properly formed CSV files and display
           position of first offending byte if not
*/

#include <csv.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int csvvalid(int argc, char* argv[]) {
    FILE* fp;
    int i;
    struct csv_parser p;
    char buf[1024];
    size_t bytes_read;
    size_t pos;
    size_t retval;

    if (argc < 2) {
        fprintf(stderr, "Usage: csvvalid files\n");
        exit(EXIT_FAILURE);
    }

    if (csv_init(&p, CSV_STRICT) != 0) {
        fprintf(stderr, "Failed to initialize csv parser\n");
        exit(EXIT_FAILURE);
    }

    for (i = 1; i < argc; i++) {
        pos = 0;
        fp = fopen(argv[i], "rb");
        if (!fp) {
            fprintf(stderr, "Failed to open %s: %s, skipping\n", argv[i], strerror(errno));
            continue;
        }
        while ((bytes_read = fread(buf, 1, 1024, fp)) > 0) {
            if ((retval = csv_parse(&p, buf, bytes_read, NULL, NULL, NULL)) != bytes_read) {
                if (csv_error(&p) == CSV_EPARSE) {
                    printf("%s: malformed at byte %lu\n", argv[i], (unsigned long)pos + retval + 1);
                    goto end;
                } else {
                    printf("Error while processing %s: %s\n", argv[i], csv_strerror(csv_error(&p)));
                    goto end;
                }
            }
            pos += bytes_read;
        }
        printf("%s well-formed\n", argv[i]);

    end:
        fclose(fp);
        csv_fini(&p, NULL, NULL, NULL);
        pos = 0;
    }

    csv_free(&p);
    return EXIT_SUCCESS;
}

#endif // CSVVALID

void test_csv() {
    csvtest();
}