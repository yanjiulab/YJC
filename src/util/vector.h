/*
 * Generic vector interface header.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; see the file COPYING; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* struct for vector */
struct _vector {
    unsigned int active;  /* number of active slots */
    unsigned int alloced; /* number of allocated slot */
    unsigned int count;
    void **index; /* index to data */
};
typedef struct _vector *vector;

#define VECTOR_MIN_SIZE 1

/* (Sometimes) usefull macros.  This macro convert index expression to
 array expression. */
/* Reference slot at given index, caller must ensure slot is active */
#define vector_slot(V, I) ((V)->index[(I)])
/* Number of active slots.
 * Note that this differs from vector_count() as it the count returned
 * will include any empty slots
 */
#define vector_active(V) ((V)->active)

/* List-like operation */
// int vector_set(vector v, void *val);

/* Prototypes */
extern vector vector_init(unsigned int size);
extern void vector_ensure(vector v, unsigned int num);
extern int vector_empty_slot(vector v);
extern int vector_set(vector v, void *val);
extern int vector_set_index(vector v, unsigned int i, void *val);
extern int vector_insert(vector v, unsigned int i, void *val);
extern void vector_unset(vector v, unsigned int i);
extern void vector_unset_value(vector v, void *val);
extern void vector_remove(vector v, unsigned int ix);
static inline unsigned int vector_count(vector v) { return v->count; }
extern void *vector_lookup(vector, unsigned int);
extern void *vector_lookup_ensure(vector, unsigned int);
extern void vector_free(vector v);
extern vector vector_copy(vector v);
extern void vector_compact(vector v);
extern void vector_to_array(vector v, void ***dest, int *argc);
extern vector array_to_vector(void **src, int argc);

/* Stack-like operation */
#define vector_first(v) vector_slot(v, 0)
#define vector_last(v) vector_slot(v, vector_active(v) - 1)
extern void vector_push(vector v, void *val);
extern void *vector_pop(vector v);

/* Advanced operation */
void vector_swap(vector v, unsigned int i, unsigned int j);
void vector_reverse(vector v);

int vector_str_cmp(const void *p1, const void *p2);
int vector_int_cmp(const void *p1, const void *p2);
int vector_double_cmp(const void *p1, const void *p2);

void vector_sort(vector v, __compar_fn_t fn);

#define vector_foreach(v, var, iter) \
    if ((v)->active > 0)             \
        for ((iter) = 0; (iter) < (v)->active && (((var) = (v)->index[(iter)]), 1); ++(iter))
#define vector_foreach_rev(v, var, iter) \
    if ((v)->active > 0)                 \
        for ((iter) = (v)->active - 1; (iter) >= 0 && (((var) = (v)->index[(iter)]), 1); --(iter))

#ifdef __cplusplus
}
#endif

#endif /* _ZEBRA_VECTOR_H */
