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
    int n = ini_sget(config, "database", "server", NULL, &server);
    n = ini_sget(config, "database", "port", "%d", &port);
    printf("server: %s:%d\n", server, port);

    int new_port = 8080;
    ini_sset(config, "database", "port", "%d", new_port);
    ini_sset(config, "owner", "organization", "%s", "Apple Inc.");
    ini_free(config);
}