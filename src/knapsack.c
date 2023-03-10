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

    // for (int i = 0; i < sack->num_objects; i++)
    //     printf("(%d, %f) ", sorted_by_density[i], sack->densities[sorted_by_density[i]]);
    // printf("\n");

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

    // for (int i = 0; i < sack->num_objects; i++)
    //     printf("(%d, %f) ", sorted_by_density[i], sack->densities[sorted_by_density[i]]);
    // printf("\n");

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

int knapsack_solve(knapsack_t* sack) {
    // Initialisation
    int bound = sack->initial_bound;
    int eval = sack->initial_eval;
    layer_t* stack = (layer_t*)malloc(sizeof(layer_t) * sack->num_objects);

    // L'heuristique est-elle l'optimal ?
    if (bound >= eval)
        return bound;

    // Push de la première valeur
    int depth = 0;
    stack[depth].obj = sack->sorted_by_density[depth];
    stack[depth].quantity = 0;
    while (depth >= 0) {
        printf("depth = %d \n", depth);
        if (stack[depth].quantity < sack->max_quantities[stack[depth].obj]) {
            // Explore wider
            int obj = stack[depth].obj + 1;
            int quantity = stack[depth].quantity;

            printf("x%d = %d \n", obj, quantity);
            glp_set_col_bnds(sack->pb, obj, GLP_FX, quantity, quantity);
            glp_simplex(sack->pb, NULL);
            double eval = glp_get_obj_val(sack->pb);
            printf("Eval = %f \n", eval);

            // Vérifie si la solution est exacte
            int exact = 1;
            for (int i = 0; i < sack->num_objects; i++) {
                double tmp = glp_get_col_prim(sack->pb, i + 1);
                double _sink;
                double fracpart = modf(tmp, &_sink);
                exact = exact && (fracpart < 1e-10);
            }
            printf("Exact = %d \n", exact);
            if (exact) {
                bound = (int)eval;
                continue;
            }

            if (eval <= bound) {
                // Explore wider
                stack[depth].quantity++;
                continue;
            } else if (depth >= sack->num_objects) {
                // C'est une feuille
                printf("C'est une feuille!\n");
                // TODO
                exit(EXIT_FAILURE);
            } else {
                // Explore deeper

                // Push new value to stack
                depth++;
                stack[depth].obj = sack->sorted_by_density[depth];
                stack[depth].quantity = 0;
            }
        } else {
            // Pop
            int obj = stack[depth].obj + 1;
            glp_set_col_bnds(sack->pb, obj, GLP_LO, 0, 0);
            depth--;
        }
    }

    // Clean-up
    free(stack);

    return bound;
}

glp_prob* knapsack_build_matrix(knapsack_t* sack, int depth, layer_t* stack) { return NULL; }
