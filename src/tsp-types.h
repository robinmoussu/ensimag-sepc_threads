#ifndef _TSP_TYPES_H
#define _TSP_TYPES_H

#define MAX_TOWNS 30

typedef int tsp_distance_matrix_t [MAX_TOWNS*MAX_TOWNS];
typedef int tsp_path_t [MAX_TOWNS];

#define MAXX    100
#define MAXY    100

/** Retourne la distance
 */
int get_distance(int x, int y);
void set_distance(int x, int y, int new_distance);

long int get_myseed();

#endif
