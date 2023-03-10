#ifndef __KNAPSACK_H__
#define __KNAPSACK_H__

#include <glpk.h>
#include <stdlib.h>

// Structure qui contient toutes les données nécéssaires au fonctionnement
// du sac à dos
typedef struct {
    // GLPK pb
    glp_prob* pb;
    int *rows, *cols;
    double* constraints;
    // Formulation
    int num_objects;
    int max_volume;
    int* volumes;
    int* values;
    int read; // Flag à 1 s'il faut désaouller volumes et values
    // Heuristiques
    double* densities;
    int initial_bound;
    double initial_eval;
    int* max_quantities;
    int* sorted_by_density;
} knapsack_t;

// Structure qui représente un niveau de l'algorithme de Branch & Bound
typedef struct {
    int obj;      // Indice de l'objet incrémenté
    int quantity; // Quantité prise de l'objet
} layer_t;

// Crée un sac à dos avec les données du problème
void knapsack_new(knapsack_t* sack, int num_objects, int max_volume, int* values, int* volumes);

// Détruit un sac à dos
void knapsack_drop(knapsack_t* sack);

// Calcule les densités (qualité/prix) de chaque objets
double* knapsack_densities(knapsack_t* sack);

// Renvoie l'indice de l'objet avec la plus grande densité
int knapsack_best_density(knapsack_t* sack);

// Calcule la quantité maximale possible pour chaque objet
int* knapsack_max_quantities(knapsack_t* sack);

// Trie les indices des objets par densités croissantes
int* knapsack_sort_by_density(knapsack_t* sack);

// Heuristique pour obtenir la borne initiale
int knapsack_initial_bound(knapsack_t* sack);

// Affiche le contenu de la structure sac à dos pour de debug
void knapsack_print(knapsack_t* sack);

// Lis un problème du sac à dos à partir d'un fichier texte
void knapsack_read(knapsack_t* sack, char* filepath);

// Résout le problème du sac à dos
int knapsack_solve(knapsack_t* sack);

// Construit le problème linéaire associé au problème initial en plus
// Des valeurs des variables déjà fixé
glp_prob* knapsack_build_matrix(knapsack_t* sack, int depth, layer_t* stack);

#endif /* __KNAPSACK_H__ */
