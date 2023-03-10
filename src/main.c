#include <glpk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "knapsack.h"

int main(int argc, char** argv) {
    knapsack_t sack;
    solution_t sol;
    char* filepath = NULL;

    if (argc > 1) {
        filepath = argv[1];
        knapsack_debug(0);
        if (argc > 2 && (strcmp("-d", argv[2]) == 0))
            knapsack_debug(1);
    } else {
        printf("Usage: \n");
        printf("%s [pb_file] [-d ?]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    knapsack_read(&sack, filepath);

    printf("Knapsack = ");
    knapsack_print(&sack);

    sol = knapsack_solve(&sack);

    printf("Optimum = ");
    solution_print(&sol);

    knapsack_drop(&sack);
    solution_drop(&sol);

    return EXIT_SUCCESS;
}