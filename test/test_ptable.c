#include "ptable.h"
#include "test.h"

void test_ptable() {
    // struct ptable t;
    ptable_t t = ptable_new();

    ptable_init(t, "Name", "%s", "Age", "%d", "Score", "%.2f", NULL);

    ptable_add(t,
               "Amet fugiat commodi eligendi possimus harum earum. "
               "Sequi quidem ab commodi tempore mollitia provident. "
               "Iusto incidunt consequuntur rem eligendi illum. "
               "Nisi odit soluta dolorum vero enim neque id. Hic magni? "
               "foo bar baz",
               36, 3.1);
    ptable_add(t,
               "Ametfugiatcommodieligendipossimusharumearum."
               "Sequiquidemabcommoditemporemollitiaprovident."
               "Iustoinciduntconsequunturremeligendiillum."
               "Nisioditsolutadolorumveroenimnequeid.Hicmagni?"
               "foobarbaz",
               36, 3.1);
    ptable_add(t, "Ξεσκεπάζωτὴνψυχοφθόραβδελυγμία", 36, 3.1);
    ptable_add(t, "Ξεσκεπάζω τὴν ψυχοφθόρα βδελυγμία", 36, 3.1);
    ptable_add(t, "Bob", 18, 1.3123);
    ptable_add(t, "Alice", 202222222, 6.43);
    ptable_add(t, "Roger", 18, 12.45);
    ptable_add(t, "Larry", 59, 12.52);
    ptable_add(t, "Ё Ђ Ѓ Є Ѕ І Ї Ј Љ", 21, 14.12312312);

    ptable_print(t, 60, stdout);

    ptable_free(t);
}