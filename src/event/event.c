<<<<<<< HEAD


struct event_base *event_base_new(void) {
    struct event_base *base = NULL;
    struct event_config *cfg = event_config_new();
    if (cfg) {
        base = event_base_new_with_config(cfg);
        event_config_free(cfg);
    }
    return base;
}

int demo() {
    struct event_base *base = event_base_new();
    struct event listen_ev;
    event_set(&listen_ev, fd, EV_READ | EV_PERSIST, on_accept, NULL);
    event_base_set(base, &listen_ev);
    
    event_add(&listen_ev, NULL);
    event_base_dispatch(base);
    // not exec
    event_del(&listen_ev);
    event_base_free(base);
}
=======
#include "event.h"
>>>>>>> f500299508943b2b9562d1c6a5c23c8dbc95f242
