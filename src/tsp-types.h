#ifndef _TSP_TYPES_H
#define _TSP_TYPES_H

#define MAX_TOWNS 30

typedef int tsp_distance_matrix_t [MAX_TOWNS] [MAX_TOWNS];
typedef int tsp_path_t [MAX_TOWNS];

#define MAXX    100
#define MAXY    100

/** tableau des distances */
extern tsp_distance_matrix_t distance;

/** parametres du programme */
extern int nb_towns;
extern long int myseed;

#endif
