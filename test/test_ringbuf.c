#include <assert.h>
#include <stdio.h>

#include "ringbuf.h"
#include "test.h"

#define BUFSIZE 4096

void test_ringbuf() {
    struct ringbuf *soil = ringbuf_new(BUFSIZE);

    printf("Validating reset on empty buffer...\n");
    ringbuf_reset(soil);

    /* put one byte */
    printf("Validating write...\n");
    uint8_t walnut = 47;
    assert(ringbuf_put(soil, &walnut, sizeof(walnut)) == 1);

    /* validate read limitations */
    printf("Validating read limits...\n");
    uint8_t nuts[2];
    assert(ringbuf_get(soil, &nuts, sizeof(nuts)) == 1);
    ringbuf_reset(soil);

    /* copy stack garbage to buffer */
    printf("Validating big write...\n");
    uint8_t compost[BUFSIZE];
    assert(ringbuf_put(soil, &compost, sizeof(compost)) == BUFSIZE);
    assert(soil->start == 0);
    assert(soil->end == 0);

    /* read 15 bytes of garbage */
    printf("Validating read...\n");
    assert(ringbuf_get(soil, &compost, 15) == 15);
    assert(soil->start == 15);
    assert(soil->end == 0);

    printf("Validating wraparound...\n");
    assert(ringbuf_put(soil, &compost[BUFSIZE / 2], 10) == 10);
    assert(soil->start == 15);
    assert(soil->end == 10);

    printf("Validating size limits...\n");
    assert(ringbuf_put(soil, &compost, 15) == 5);

    printf("Validating big read...\n");
    assert(ringbuf_get(soil, &compost, BUFSIZE) == BUFSIZE);
    assert(soil->empty == true);
    assert(soil->start == soil->end);
    assert(soil->start == 15);

    printf("Validating empty read...\n");
    assert(ringbuf_get(soil, &compost, 1) == 0);

    printf("Validating wipe...\n");
    memset(&compost, 0x00, sizeof(compost));
    ringbuf_wipe(soil);
    assert(memcmp(&compost, soil->data, sizeof(compost)) == 0);

    /* validate simple data encode / decode */
    const char *organ = "seed";
    printf("Encoding: '%s'\n", organ);
    assert(ringbuf_put(soil, organ, strlen(organ)) == 4);
    char water[strlen(organ) + 1];
    assert(ringbuf_get(soil, &water, strlen(organ)) == 4);
    water[strlen(organ)] = '\0';
    printf("Retrieved: '%s'\n", water);

    /* validate simple data encode / decode across ring boundary */
    soil->start = soil->size - 2;
    soil->end = soil->start;
    const char *phloem = "root";
    printf("Encoding: '%s'\n", phloem);
    printf("start:%d, end:%d\n", soil->start, soil->end);
    assert(ringbuf_put(soil, phloem, strlen(phloem)) == 4);
    char xylem[strlen(phloem) + 1];
    assert(ringbuf_get(soil, &xylem, 100) == 4);
    xylem[strlen(phloem)] = '\0';
    printf("Retrieved: '%s'\n", xylem);
    printf("start:%d, end:%d\n", soil->start, soil->end);

    ringbuf_wipe(soil);

    /* validate simple data peek across ring boundary */
    soil->start = soil->size - 2;
    soil->end = soil->start;
    const char *cytoplasm = "tree";
    printf("Encoding: '%s'\n", cytoplasm);
    printf("start:%d, end:%d\n", soil->start, soil->end);
    assert(ringbuf_put(soil, cytoplasm, strlen(cytoplasm)) == 4);
    char chloroplast[strlen(cytoplasm) + 1];
    assert(ringbuf_peek(soil, 2, &chloroplast[0], 100) == 2);
    assert(ringbuf_peek(soil, 0, &chloroplast[2], 2) == 2);
    chloroplast[strlen(cytoplasm)] = '\0';
    assert(!strcmp(chloroplast, "eetr"));
    printf("Retrieved: '%s'\n", chloroplast);
    printf("start:%d, end:%d\n", soil->start, soil->end);

    printf("Deleting...\n");
    ringbuf_del(soil);

    printf("Creating new buffer...\n");
    soil = ringbuf_new(15);
    soil->start = soil->end = 7;

    /* validate data encode of excessive data */
	const char *twenty = "vascular plants----";
	char sixteen[16];
	printf("Encoding: %s\n", twenty);
	assert(ringbuf_put(soil, twenty, strlen(twenty)) == 15);
	assert(ringbuf_get(soil, sixteen, 20));
	sixteen[15] = '\0';
	printf("Retrieved: %s\n", sixteen);
	assert(!strcmp(sixteen, "vascular plants"));

	printf("Deleting...\n");
	ringbuf_del(soil);

	printf("Done.\n");
}