#ifndef TSP_PRINT_H
#define TSP_PRINT_H

#include "tsp-types.h"
#include "tsp-genmap.h"

/* impression tableau des distances, pour vérifier au besoin */
void print_distance_matrix (void);

/* Affichage d'une solution possible. */
void print_solution (tsp_path_t path, int len);

void print_solution_svg (tsp_path_t path, int len);

#endif
