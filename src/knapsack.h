#ifndef __KNAPSACK_H__
#define __KNAPSACK_H__

#include <glpk.h>
#include <stdlib.h>

static int KNAPSACK_DEBUG = 1;

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
    int* initial_solution;
    double initial_eval;
    int* max_quantities;
    int* sorted_by_density;
} knapsack_t;

// Structure qui contient la solution du problème d'optimisation du sac à dos
// Par méthode Branch & Bound
typedef struct {
    int num_objects;
    int value;
    int* quantities;
} solution_t;

// Active/désactive le mode debug pour le solver
void knapsack_debug(int mode);

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
solution_t knapsack_solve(knapsack_t* sack);

// Evalue un branche (feuille ou branche partielle) dans
// l'algorithme de Branch & Bound
double knapsack_eval_branch(knapsack_t* sack, int depth, int* quantities);

// Vérifie si une relaxation par le simplexe est exacte
int knapsack_is_relax_exact(knapsack_t* sack, int depth);

// Affiche la solution obtenue par le solveur
void solution_print(solution_t* solution);

// Détruit la mémoire allouée pour la solution
void solution_drop(solution_t* solution);

#endif /* __KNAPSACK_H__ */
