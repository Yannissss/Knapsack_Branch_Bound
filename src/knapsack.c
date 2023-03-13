#include "knapsack.h"

#include <glpk.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void knapsack_new(knapsack_t* sack, int num_objects, int max_volume, int* values, int* volumes) {
    glp_prob* pb;
    int *rows, *cols;
    double* constraints;
    char tmp[8];

    pb = glp_create_prob();             /*creation d'un objet probleme*/
    glp_set_prob_name(pb, "Sac à dos"); /*definition d'un nom symbolique pour le probleme*/

    glp_set_obj_dir(pb, GLP_MAX); /*definition du type d'optimisation, ici maximisation*/
    glp_add_rows(pb, 1);

    glp_set_row_name(pb, 1,
                     "Volume du sac"); /*definition d'un nom symbolique pour la contrainte 1*/
    glp_set_row_bnds(pb, 1, GLP_UP, 0.0, (double)max_volume);

    glp_add_cols(pb, num_objects);
    for (size_t i = 1; i <= num_objects; i++) {
        sprintf(tmp, "x%ld", i);

        glp_set_col_name(pb, i, tmp);
        glp_set_col_bnds(pb, i, GLP_LO, 0.0, 0.0);
        glp_set_obj_coef(pb, i, (double)values[i - 1]);
    }

    rows = malloc(sizeof(int) * (1 + num_objects));
    cols = malloc(sizeof(int) * (1 + num_objects));
    constraints = malloc(sizeof(double) * (1 + num_objects));
    for (int i = 0; i <= num_objects; i++) {
        rows[i] = 1;
        cols[i] = i;
        constraints[i] = (double)volumes[i - 1];
    }

    glp_term_out(GLP_OFF);
    glp_load_matrix(pb, num_objects, rows, cols, constraints);

    // Problème GLPK
    sack->pb = pb;
    sack->rows = rows;
    sack->cols = cols;
    sack->constraints = constraints;

    // Données initiales
    sack->num_objects = num_objects;
    sack->max_volume = max_volume;
    sack->volumes = volumes;
    sack->values = values;
    sack->read = 0;

    // Heuristiques
    sack->densities = knapsack_densities(sack);

    glp_simplex(sack->pb, NULL);
    sack->initial_eval = glp_get_obj_val(pb);

    sack->max_quantities = knapsack_max_quantities(sack);
    sack->sorted_by_density = knapsack_sort_by_density(sack);

    sack->initial_solution = (int*)malloc(sizeof(int) * num_objects);
    sack->initial_bound = knapsack_initial_bound(sack);
}

void knapsack_drop(knapsack_t* sack) {
    glp_delete_prob(sack->pb);
    free(sack->rows);
    free(sack->cols);
    free(sack->constraints);
    free(sack->densities);
    free(sack->max_quantities);
    free(sack->sorted_by_density);
    free(sack->initial_solution);
    if (sack->read) {
        free(sack->volumes);
        free(sack->values);
    }
}

double* knapsack_densities(knapsack_t* sack) {
    double* densities = (double*)malloc(sizeof(double) * sack->num_objects);
    for (int i = 0; i < sack->num_objects; i++)
        densities[i] = ((double)sack->values[i]) / ((double)sack->volumes[i]);
    return densities;
}

int knapsack_best_density(knapsack_t* sack) {
    int j = -1;
    double max_density = -1.f;

    for (int i = 0; i < sack->num_objects; i++) {
        double density = sack->densities[i];
        if (density > max_density) {
            max_density = density;
            j = i;
        }
    }

    return j;
}

int* knapsack_max_quantities(knapsack_t* sack) {
    int max_volume = (int)sack->max_volume;

    int* max_quantities = (int*)malloc(sizeof(int) * sack->num_objects);
    for (int i = 0; i < sack->num_objects; i++) {
        max_quantities[i] = max_volume / (int)sack->volumes[i];
    }

    return max_quantities;
}

int* knapsack_sort_by_density(knapsack_t* sack) {
    int* sorted_by_density = (int*)malloc(sizeof(int) * sack->num_objects);

    // Remplissage du tableau
    for (int i = 0; i < sack->num_objects; i++)
        sorted_by_density[i] = i;

    // Tri par insertion
    int j;
    for (int i = 1; i < sack->num_objects; i++) {
        int curr = sack->densities[sorted_by_density[i]];
        j = i;
        while ((j > 0) && sack->densities[sorted_by_density[j - 1]] < curr) {
            sorted_by_density[j] = sorted_by_density[j - 1];
            j--;
        }
        sorted_by_density[j] = i;
    }

    return sorted_by_density;
}

int knapsack_initial_bound(knapsack_t* sack) {
    int volume = sack->max_volume;
    int value = 0;

    for (int j = 0; j < sack->num_objects; j++) {
        int i = sack->sorted_by_density[j];
        int quantity = volume / sack->volumes[i];
        volume -= quantity * sack->volumes[i];
        value += quantity * sack->values[i];
        sack->initial_solution[i] = quantity;
    }

    return value;
}

void print_veci(int* v, int len) {
    printf("[");
    for (int i = 0; i < len - 1; i++)
        printf("%d, ", v[i]);
    printf("%d]", v[len - 1]);
}

void print_vecd(double* v, int len) {
    printf("[");
    for (int i = 0; i < len - 1; i++)
        printf("%.1f, ", v[i]);
    printf("%.1f]", v[len - 1]);
}

void knapsack_print(knapsack_t* sack) {
    printf("knapsack_t {\n");
    printf("    num_objects: %d,\n", sack->num_objects);
    printf("    max_volume: %d,\n", sack->max_volume);
    printf("    volumes: ");
    print_veci(sack->volumes, sack->num_objects);
    printf(",\n");
    printf("    values: ");
    print_veci(sack->values, sack->num_objects);
    printf(",\n");
    printf("    densities: ");
    print_vecd(sack->densities, sack->num_objects);
    printf(",\n");
    printf("    initial_bound: %d,\n", sack->initial_bound);
    printf("    initial_eval: %.1f,\n", sack->initial_eval);
    printf("    max_quantities: ");
    print_veci(sack->max_quantities, sack->num_objects);
    printf(",\n");
    printf("    sorted_by_density: ");
    print_veci(sack->sorted_by_density, sack->num_objects);
    printf("\n");
    printf("}\n");
}

void knapsack_read(knapsack_t* sack, char* filepath) {
    // CODE tiré du TP1 - INFO CHPS805 (Pierre Delisle)
    /*Lit les donnees d'un probleme a partir d'un fichier specifie*/
    /*et les place dans la structure prob*/

    FILE* fichier;
    char ch_temp[20], signe[3];
    int j;

    int num_objects;
    int max_volume;
    int* values;
    int* volumes;

    if ((fichier = fopen(filepath, "r"))) {
        fscanf(fichier, "%s", ch_temp);

        fscanf(fichier, "%d", &num_objects);
        fscanf(fichier, "%s", ch_temp);

        int tmp;
        fscanf(fichier, "%d", &tmp);
        if (tmp != 1) {
            fprintf(stderr, "Plus d'une contrainte sur le sac à dos! \n");
            exit(EXIT_FAILURE);
        }

        fscanf(fichier, "%s", ch_temp);
        if (strcmp(ch_temp, "max")) {
            fprintf(stderr, "Le sac à dos ne fonctionne que en maximisation \n");
            exit(EXIT_FAILURE);
        }
        volumes = (int*)malloc(sizeof(int) * num_objects);
        values = (int*)malloc(sizeof(int) * num_objects);

        // Récupération valeurs
        for (j = 0; j < num_objects; j++) {
            fscanf(fichier, "%s", ch_temp);

            double tmp;
            fscanf(fichier, "%lf", &tmp);
            values[j] = (int)tmp;

            if (!strcmp(ch_temp, "-")) {
                fprintf(stderr, "Valeurs négatives non supportées \n");
                exit(EXIT_FAILURE);
            }
            fscanf(fichier, "%s", ch_temp);
        }

        // Récupération volumes
        for (j = 0; j < num_objects; j++) {
            fscanf(fichier, "%s", ch_temp);

            double tmp;
            fscanf(fichier, "%lf", &tmp);
            volumes[j] = (int)tmp;

            if (!strcmp(signe, "-")) {
                fprintf(stderr, "Valeurs négatives non supportées \n");
                exit(EXIT_FAILURE);
            }

            fscanf(fichier, "%s", ch_temp);
        }

        fscanf(fichier, "%s", ch_temp);
        if (!strcmp(signe, "<=")) {
            fprintf(stderr, "Erreur pas de volume maximal! \n");
            exit(EXIT_FAILURE);
        }

        // Récupération volume maximal
        fscanf(fichier, "%d", &tmp);
        max_volume = (int)tmp;

        fclose(fichier);
    } else {
        fprintf(stderr, "Problème lecture de fichier: %s \n", filepath);
        exit(EXIT_FAILURE);
    }

    knapsack_new(sack, num_objects, max_volume, values, volumes);
    sack->read = 1;
}

solution_t knapsack_solve(knapsack_t* sack) {
    // Initialisation
    int bound = sack->initial_bound;
    int eval = sack->initial_eval;
    int depth = 0;
    int* quantities = (int*)malloc(sizeof(int) * sack->num_objects);

    solution_t solution;
    solution.num_objects = sack->num_objects;

    // Recopie de la solution initial
    solution.value = bound;
    solution.quantities = (int*)malloc(sizeof(int) * sack->num_objects);
    for (int i = 0; i < sack->num_objects; i++)
        solution.quantities[i] = sack->initial_solution[i];

    // L'heuristique est-elle l'optimal ?
    if (bound >= eval) {
        return solution;
    }

    // Push de la première valeur
    quantities[depth] = -1;

    while (depth >= 0) {
        // Exploration en largeur
        quantities[depth]++;
        int max_quantity = sack->max_quantities[sack->sorted_by_density[depth]];
        int quantity = quantities[depth];

        // Calcul du volume occupé
        int volume = 0;
        for (int j = 0; j <= depth; j++) {
            int i = sack->sorted_by_density[j];
            volume += sack->volumes[i] * quantities[j];
        }

        if (KNAPSACK_DEBUG) {
            printf("----------- \n");
            printf("depth = %d \n", depth);
            printf("volume = %d \n", volume);
            for (int j = 0; j <= depth; j++) {
                printf("x%d = %d, ", sack->sorted_by_density[j] + 1, quantities[j]);
            }
            printf("\n");
        }

        // Vérifie si la quantité fixée est possible
        // Et s'il reste de la place dans le sac à dos
        if (quantity <= max_quantity && volume <= sack->max_volume) {

            // Evaluation
            double eval = knapsack_eval_branch(sack, depth, quantities);

            // Vérifie si la solution est exacte
            int exact = knapsack_is_relax_exact(sack, depth);

            if (KNAPSACK_DEBUG) {
                printf("Bound = %d \n", bound);
                printf("Eval = %f \n", eval);
                printf("Exact = %d \n", exact);
            }

            if (exact && (int)eval > bound) {
                // Mise à jour de la borne et exploration en largeur
                bound = (int)eval;

                // Mise à jour de la solution optimale
                solution.value = bound;
                for (int j = 0; j < sack->num_objects; j++) {
                    if (depth >= sack->num_objects - 1) {
                        // On est sur une feuille
                        solution.quantities[j] = quantities[sack->sorted_by_density[j]];
                    } else {
                        // On a fait un simplex
                        solution.quantities[j] = glp_get_col_prim(sack->pb, j + 1);
                    }
                }

                continue;
            } else if (eval <= bound) {
                // Exploration en largeur
                // Pas de mise à jour de la borne
                continue;
            } else {
                // Exploration en profondeur

                // Si on est sur une feuille non
                if (depth >= sack->num_objects - 1)
                    // Exploration en largeur
                    continue;
                else {
                    if (KNAPSACK_DEBUG)
                        printf("=> GO DOWN \n");
                    depth++;
                    quantities[depth] = -1;
                }
            }

        } else {
            if (KNAPSACK_DEBUG)
                printf("=> GO UP \n");
            glp_set_col_bnds(sack->pb, depth + 1, GLP_LO, 0, 0);
            depth--;
        }
    }

    // Clean-up
    free(quantities);

    return solution;
}

double knapsack_eval_branch(knapsack_t* sack, int depth, int* quantities) {
    if (depth >= sack->num_objects - 1) { // On est sur une feuille
        double eval = 0.0;

        for (int j = 0; j < sack->num_objects; j++) {
            int i = sack->sorted_by_density[j];
            eval += quantities[j] * sack->values[i];
        }

        return eval;
    } else { // Utilisation du simplexe
        int quantity = quantities[depth];

        glp_set_col_bnds(sack->pb, depth + 1, GLP_FX, quantity, quantity);
        glp_simplex(sack->pb, NULL);

        return glp_get_obj_val(sack->pb);
    }
}

int knapsack_is_relax_exact(knapsack_t* sack, int depth) {
    double tmp, sink;
    int exact = 1;

    if (depth >= sack->num_objects - 1) {
        exact = 1; // C'est une feuille donc oui
    } else {
        for (int i = 0; i < sack->num_objects; i++) {
            tmp = glp_get_col_prim(sack->pb, i + 1);
            double fracpart = modf(tmp, &sink);
            exact = exact && (fracpart < 1e-10);
        }
    }

    return exact;
}

void knapsack_debug(int mode) { KNAPSACK_DEBUG = mode; }

void solution_print(solution_t* solution) {
    printf("solution_t {\n");
    printf("    value: %d, \n", solution->value);
    printf("    quantities: ");
    print_veci(solution->quantities, solution->num_objects);
    printf("\n}\n");
}

void solution_drop(solution_t* solution) { free(solution->quantities); }
