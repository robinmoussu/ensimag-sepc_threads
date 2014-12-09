#ifndef TSP_GENMAP_H
#define TSP_GENMAP_H

/** nombre de villes */
extern int nb_towns;
typedef struct {
    int x, y ;
} coor_t ;

typedef coor_t *coortab_t;
coortab_t towns;

/** initialisation du tableau des distances */
void genmap ();

int* get_cutprefix();

#endif
