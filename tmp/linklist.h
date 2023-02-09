/* Generic linked list
 * Copyright (C) 1997, 2000 Kunihiro Ishiguro
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
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef LIST_H
#define LIST_H

/* listnodes must always contain data to be valid. Adding an empty node
 * to a list is invalid
 */
struct listnode {
    struct listnode *next;
    struct listnode *prev;

    /* private member, use getdata() to retrieve, do not access directly */
    void *data;
};

typedef struct list {
    struct listnode *head;
    struct listnode *tail;

    /* invariant: count is the number of listnodes in the list */
    unsigned int count;

    /*
     * Returns -1 if val1 < val2, 0 if equal?, 1 if val1 > val2.
     * Used as definition of sorted for listnode_add_sort
     */
    int (*cmp)(void *val1, void *val2);

    /* callback to free user-owned data when listnode is deleted. supplying
     * this callback is very much encouraged!
     */
    void (*del)(void *val);
} list;

#define listnextnode(X) ((X) ? ((X)->next) : NULL)
#define listhead(X) ((X) ? ((X)->head) : NULL)
#define listtail(X) ((X) ? ((X)->tail) : NULL)
#define listcount(X) ((X)->count)
#define list_isempty(X) ((X)->head == NULL && (X)->tail == NULL)
#define listgetdata(X) (X)->data

/* Prototypes. */
extern struct list *list_new(void); /* encouraged: set list.del callback on new lists */
extern void list_free(struct list *);

extern void listnode_add(struct list *, void *);
extern void listnode_add_sort(struct list *, void *);
extern void listnode_add_after(struct list *, struct listnode *, void *);
extern struct listnode *listnode_add_before(struct list *, struct listnode *, void *);
extern void listnode_move_to_tail(struct list *, struct listnode *);
extern void listnode_delete(struct list *, void *);
extern struct listnode *listnode_lookup(struct list *, void *);
extern void *listnode_head(struct list *);

extern void list_delete(struct list *);
extern void list_delete_all_node(struct list *);
extern void list_dump(struct list *, const char *);

/* List iteration macro.
 * Usage: for (ALL_LIST_ELEMENTS (...) { ... }
 * It is safe to delete the listnode using this macro.
 */
#define ALL_LIST_ELEMENTS(list, node, nextnode, data)                           \
    (node) = listhead(list), ((data) = NULL);                                   \
    (node) != NULL && ((data) = listgetdata(node), (nextnode) = node->next, 1); \
    (node) = (nextnode), ((data) = NULL)

#define list_foreach(list, node, nextnode, data) for (ALL_LIST_ELEMENTS(list, node, nextnode, data))

/* read-only list iteration macro.
 * Usage: as per ALL_LIST_ELEMENTS, but not safe to delete the listnode Only
 * use this macro when it is *immediately obvious* the listnode is not
 * deleted in the body of the loop. Does not have forward-reference overhead
 * of previous macro.
 */
#define ALL_LIST_ELEMENTS_RO(list, node, data)         \
    (node) = listhead(list), ((data) = NULL);          \
    (node) != NULL && ((data) = listgetdata(node), 1); \
    (node) = listnextnode(node), ((data) = NULL)

#endif /* LIST_H */
