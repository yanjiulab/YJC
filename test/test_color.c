#include "color.h"
#include "test.h"

void test_color() {
    printf(BLACK "BLACK        " NORMAL);
    printf(RED "RED          " NORMAL);
    printf(GREEN "GREEN        " NORMAL);
    printf(YELLOW "YELLOW       " NORMAL);
    printf(BLUE "BLUE         " NORMAL);
    printf(PURPLE "PURPLE       " NORMAL);
    printf(CYAN "CYAN         " NORMAL);
    printf(GREY "GREY         " NORMAL);
    printf("\n");
    printf(L_DKGREY "L_DKGREY     " NORMAL);
    printf(L_RED "L_RED        " NORMAL);
    printf(L_GREEN "L_GREEN      " NORMAL);
    printf(L_YELLOW "L_YELLOW     " NORMAL);
    printf(L_BLUE "L_BLUE       " NORMAL);
    printf(L_PURPLE "L_PURPLE     " NORMAL);
    printf(L_CYAN "L_CYAN       " NORMAL);
    printf(L_GREY "L_GREY       " NORMAL);
    printf("\n");
    printf(B_DKGREY "B_DKGREY     " NORMAL);
    printf(B_RED "B_RED        " NORMAL);
    printf(B_GREEN "B_GREEN      " NORMAL);
    printf(B_ORANGE "B_ORANGE     " NORMAL);
    printf(B_BLUE "B_BLUE       " NORMAL);
    printf(B_PURPLE "B_PURPLE     " NORMAL);
    printf(B_CYAN "B_CYAN       " NORMAL);
    printf(B_GREY "B_GREY       " NORMAL);
    printf("\n");
}
