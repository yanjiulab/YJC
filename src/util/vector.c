/*
 * Generic vector interface header.
 */

#include "vector.h"

/* Initialize vector : allocate memory and return vector. */
vector vector_init(unsigned int size) {
    vector v = calloc(1, sizeof(struct _vector));

    /* allocate at least one slot */
    if (size == 0) size = 1;

    v->alloced = size;
    v->active = 0;
    v->count = 0;
    v->index = calloc(1, sizeof(void *) * size);
    return v;
}

void vector_free(vector v) {
    free(v->index);
    free(v);
}

vector vector_copy(vector v) {
    unsigned int size;
    vector new = calloc(1, sizeof(struct _vector));

    new->active = v->active;
    new->alloced = v->alloced;
    new->count = v->count;

    size = sizeof(void *) * (v->alloced);
    new->index = calloc(1, size);
    memcpy(new->index, v->index, size);

    return new;
}

/* Check assigned index, and if it runs short double index pointer */
void vector_ensure(vector v, unsigned int num) {
    if (v->alloced > num) return;
    v->index = realloc(v->index, sizeof(void *) * (v->alloced * 2));
    memset(&v->index[v->alloced], 0, sizeof(void *) * v->alloced);
    v->alloced *= 2;

    if (v->alloced <= num) vector_ensure(v, num);
}

/* This function only returns next empty slot index.  It dose not mean
   the slot's index memory is assigned, please call vector_ensure()
   after calling this function. */
int vector_empty_slot(vector v) {
    unsigned int i;

    if (v->active == v->count) return v->active;

    if (v->active == 0) return 0;

    for (i = 0; i < v->active; i++)
        if (v->index[i] == 0) return i;

    return i;
}

/* Set value to the smallest empty slot. */
int vector_set(vector v, void *val) {
    unsigned int i;

    i = vector_empty_slot(v);
    vector_ensure(v, i);

    if (v->index[i]) v->count--;
    if (val) v->count++;
    v->index[i] = val;

    if (v->active <= i) v->active = i + 1;

    return i;
}

/* Set value to specified index slot. */
int vector_set_index(vector v, unsigned int i, void *val) {
    vector_ensure(v, i);

    if (v->index[i]) v->count--;
    if (val) v->count++;
    v->index[i] = val;

    if (v->active <= i) v->active = i + 1;

    return i;
}

/* Insert value to specified index slot. */
int vector_insert(vector v, unsigned int i, void *val) {
    if (i > v->active) return -1;
    vector_ensure(v, v->active);
    memmove(&v->index[i + 1], &v->index[i], (v->active - i) * sizeof(void *));
    if (val) v->count++;
    v->index[i] = val;
    v->active++;
    if (v->active <= i) v->active = i + 1;
    return i;
}

/* Look up vector.  */
void *vector_get(vector v, unsigned int i) {
    if (i >= v->active) return NULL;
    return v->index[i];
}

/* Lookup vector, ensure it. */
void *vector_get_ensure(vector v, unsigned int i) {
    vector_ensure(v, i);
    return v->index[i];
}

/* Unset value at specified index slot. */
void vector_unset(vector v, unsigned int i) {
    if (i >= v->alloced) return;

    if (v->index[i]) v->count--;

    v->index[i] = NULL;

    if (i + 1 == v->active) {
        v->active--;
        while (i && v->index[--i] == NULL && v->active--)
            ; /* Is this ugly ? */
    }
}

void vector_remove(vector v, unsigned int ix) {
    if (ix >= v->active) return;

    if (v->index[ix]) v->count--;

    int n = (--v->active) - ix;

    memmove(&v->index[ix], &v->index[ix + 1], n * sizeof(void *));
    v->index[v->active] = NULL;
}

void vector_compact(vector v) {
    for (unsigned int i = 0; i < vector_active(v); ++i) {
        if (vector_slot(v, i) == NULL) {
            vector_remove(v, i);
            --i;
        }
    }
}

void vector_unset_value(vector v, void *val) {
    size_t i;

    for (i = 0; i < v->active; i++)
        if (v->index[i] == val) {
            v->index[i] = NULL;
            v->count--;
            break;
        }

    if (i + 1 == v->active) do
            v->active--;
        while (i && v->index[--i] == NULL);
}

void vector_to_array(vector v, void ***dest, int *argc) {
    *dest = calloc(1, sizeof(void *) * v->active);
    memcpy(*dest, v->index, sizeof(void *) * v->active);
    *argc = v->active;
}

vector array_to_vector(void **src, int argc) {
    vector v = vector_init(VECTOR_MIN_SIZE);

    for (int i = 0; i < argc; i++) vector_set_index(v, i, src[i]);
    return v;
}

/* Stack-like operation */
void vector_push(vector v, void *val) {
    unsigned int idx = vector_active(v);
    vector_ensure(v, idx);
    vector_set_index(v, idx, val);
}

void *vector_pop(vector v) {
    void *val = vector_last(v);
    vector_remove(v, vector_active(v) - 1);
    return val;
}

/* Advanced operation */
void vector_swap(vector v, unsigned int i, unsigned int j) {
    if (vector_active(v) < 2) return;
    if (i == j) return;

    void *val;
    val = v->index[i];
    v->index[i] = v->index[j];
    v->index[j] = val;
}

void vector_reverse(vector v) {
    int c = vector_active(v) / 2;
    while (c--) {
        vector_swap(v, c, vector_active(v) - (c + 1));
    }
}

int vector_ptr_cmp(const void *p1, const void *p2) { return *(int *)p1 - *(int *)p2; }

int vector_int_cmp(const void *p1, const void *p2) { return **(int **)p1 - **(int **)p2; }

int vector_str_cmp(const void *p1, const void *p2) {
    const char *str1 = *(const char **)p1;
    const char *str2 = *(const char **)p2;
    if (!str1 && !str2) return 0;
    if (!str1) return -1;
    if (!str2) return 1;
    return strcmp(str1, str2);
}

int vector_double_cmp(const void *p1, const void *p2) {
    double diff = **(double **)p1 - **(double **)p2;
    if (fabs(diff) < 1e-9)
        return 0;
    else
        return diff > 0 ? 1 : -1;
}

void vector_sort(vector v, __compar_fn_t fn) {
    if (fn == NULL)
        qsort(&v->index[0], v->active, sizeof(void *), vector_int_cmp);
    else
        qsort(&v->index[0], v->active, sizeof(void *), fn);
}

int vector_find(vector v, void *val, __compar_fn_t fn) {
    int i;
    void *var;
    vector_foreach(v, var, i) {
        if (fn) {
            if (fn(&var, &val) == 0) break;
        } else {
            if (var == val) break;
        }
    }
    if (i == v->active) i = -1;
    return i;
}