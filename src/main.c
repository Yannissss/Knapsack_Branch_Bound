#include <glpk.h>
#include <stdio.h>
#include <stdlib.h>

#include "knapsack.h"

/* Problème:
Maximiser: 15 x1 + 12 x2 + 2 x3 + 8 x4
Avec: 5 x1 + 3 x2 + 2 x3 + 4 x4 <= 14
*/
int main(void) {
    knapsack_t sack;

    // int values[4] = {15, 12, 2, 8};
    // int volumes[4] = {5, 3, 2, 4};
    // knapsack_new(&sack, 4, 14, values, volumes);

    knapsack_read(&sack, "sacks/prob_sac_1.txt");

    knapsack_print(&sack);

    int solution = knapsack_solve(&sack);
    printf("Solution = %d \n", solution);

    knapsack_drop(&sack);

    return 0;
}