#include "test.h"
#include "vrf.h"
#include "log.h"
void test_vrf() {
    struct vrf* v = vrf_get(0, vrf_get_default_name());
    log_info("vrf: %p", v);
    log_info("name: %s", v->name);
    log_info("id: %d", v->vrf_id);

    v = vrf_lookup_by_name("vrf0");
    log_info("vrf: %p", v);
    log_info("name: %s", v->name);
    log_info("id: %d", v->vrf_id);
    
}
