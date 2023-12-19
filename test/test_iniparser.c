#include "iniparser.h"
#include "test.h"

void test_iniparser() {
    // Read
    dictionary* ini = iniparser_load("config.ini");
    iniparser_dump(ini, stderr);

    int b;
    char* s;
    int i;
    double d;

    b = iniparser_getboolean(ini, "pizza:ham", -1);
    printf("Ham:       [%d]\n", b);
    b = iniparser_getboolean(ini, "pizza:mushrooms", -1);
    printf("Mushrooms: [%d]\n", b);
    b = iniparser_getboolean(ini, "pizza:capres", -1);
    printf("Capres:    [%d]\n", b);
    b = iniparser_getboolean(ini, "pizza:cheese", -1);
    printf("Cheese:    [%d]\n", b);

    s = iniparser_getstring(ini, "wine:grape", NULL);
    printf("Grape:     [%s]\n", s ? s : "UNDEF");
    i = iniparser_getint(ini, "wine:year", -1);
    printf("Year:      [%d]\n", i);
    s = iniparser_getstring(ini, "wine:country", NULL);
    printf("Country:   [%s]\n", s ? s : "UNDEF");
    d = iniparser_getdouble(ini, "wine:alcohol", -1.0);
    printf("Alcohol:   [%g]\n", d);

    // Update ini
    iniparser_set(ini, "database:server", "192.168.1.1");
    char buff[64];
    sprintf(buff, "%d ; fantastic year", 1989);
    iniparser_set(ini, "wine:year", buff);

    // Write ini to file
    FILE* ini_file;
    if ((ini_file = fopen("config.ini", "w")) == NULL) {
        fprintf(stderr, "iniparser: cannot create example.ini\n");
        return;
    }
    iniparser_dump_ini(ini, ini_file);
    fclose(ini_file);
    iniparser_freedict(ini);

    // Reload ini
    ini = iniparser_load("config.ini");
    if (ini == NULL) {
        fprintf(stderr, "cannot parse file\n");
        return -1;
    }
    iniparser_dump(ini, stdout);
    iniparser_freedict(ini);

    return 0;
}
