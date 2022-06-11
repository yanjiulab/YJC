#include "ini.h"
#include "test.h"

void test_ini() {
    ini_t *config = ini_load("config.ini");

    const char *name = ini_get(config, "owner", "name");
    if (name) {
        printf("name: %s\n", name);
    }

    const char *server = "default";
    int port = 80;

    ini_sget(config, "database", "server", NULL, &server);
    ini_sget(config, "database", "port", "%d", &port);

    printf("server: %s:%d\n", server, port);

    ini_free(config);
}