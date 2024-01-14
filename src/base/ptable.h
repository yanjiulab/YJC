/**
 * Print Pretty Table like below:
 * 
    +--------------------------------------------------------+
    | Name                               | Age       | Score |
    *--------------------------------------------------------*
    | Amet fugiat commodi eligendi       | 36        | 3.10  |
    | possimus harum earum. Sequi quidem |           |       |
    | ab commodi tempore mollitia        |           |       |
    | provident. Iusto incidunt          |           |       |
    | consequuntur rem eligendi illum.   |           |       |
    | Nisi odit soluta dolorum vero enim |           |       |
    | neque id. Hic magni? foo bar baz   |           |       |
    |------------------------------------|-----------|-------|
    | Ametfugiatcommodieligendipossimus- | 36        | 3.10  |
    | harumearum.Sequiquidemabcommodite- |           |       |
    | mporemollitiaprovident.Iustoincid- |           |       |
    | untconsequunturremeligendiillum.N- |           |       |
    | isioditsolutadolorumveroenimneque- |           |       |
    | id.Hicmagni?foobarbaz              |           |       |
    |------------------------------------|-----------|-------|
    | Ξεσκεπάζωτὴνψυχοφθόραβδελυγμία     | 36        | 3.10  |
    |------------------------------------|-----------|-------|
    | Ξεσκεπάζω τὴν ψυχοφθόρα βδελυγμία  | 36        | 3.10  |
    |------------------------------------|-----------|-------|
    | Bob                                | 18        | 1.31  |
    |------------------------------------|-----------|-------|
    | Alice                              | 202222222 | 6.43  |
    |------------------------------------|-----------|-------|
    | Roger                              | 18        | 12.45 |
    |------------------------------------|-----------|-------|
    | Larry                              | 59        | 12.52 |
    |------------------------------------|-----------|-------|
    | Ё Ђ Ѓ Є Ѕ І Ї Ј Љ                  | 21        | 14.12 |
    +--------------------------------------------------------+
 * 
 */

#ifndef PTABLE_H
#define PTABLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

struct ptable {
    size_t cols;
    size_t rows;
    size_t alloc;
    size_t *max;
    char ***data;
    char **headers;
    char *fmt;
};

typedef struct ptable *ptable_t;

ptable_t ptable_new();
bool ptable_init(struct ptable *t, ...);
bool ptable_add(struct ptable *t, ...);
bool ptable_print(struct ptable const *t, size_t maxwidth, FILE *f);
void ptable_free(struct ptable *t);

#endif