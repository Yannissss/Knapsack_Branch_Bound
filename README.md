# Méthode de séparation évaluation en C

## Compilation

Pour compiler ce projet, il vous faudra une version de LLVM >= 12.0 et une version de la bibliothèque GLPK installée.

Il vous faudra ensuite simplement taper la commande:
```bash
make
```
afin de compiler le projet.

## Exécution

L'exécution de l'application se fait simplement de la manière suivante.
```bash
./knapsack [chemin_du_probleme]
```

Les fichiers problème se trouve dans le répertoire sacks/.

## Remarques

Cette application ne supporte que la première version des fichiers de problème (à savoir celle en formulation de programme linéaire).