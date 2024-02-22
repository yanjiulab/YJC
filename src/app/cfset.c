#include "iniparser.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: ./cfset config.ini section:key val\n");
        return 0;
    }

    for (int i = 0; i < 4; i++) {
        printf("argv[%i]: %s\n", i, argv[i]);
    }

    dictionary* ini = iniparser_load(argv[1]);
    iniparser_set(ini, argv[2], argv[3]);
    // iniparser_dump(ini, stderr);

    FILE* ini_file;
    if ((ini_file = fopen(argv[1], "w")) == NULL) {
        fprintf(stderr, "iniparser: cannot create %s\n", argv[1]);
        return 0;
    }
    iniparser_dump_ini(ini, ini_file);
    fclose(ini_file);
    iniparser_freedict(ini);
    return 0;
}