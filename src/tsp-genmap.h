#ifndef TSP_GENMAP_H
#define TSP_GENMAP_H

typedef struct {
    int x, y ;
} coor_t ;

/** Retourne le nombre de villes
 */
int get_nb_towns();

/** Initialise le nombre de villes
 */
void set_nb_towns(int new_nb_towns);

/** Retourne la liste des villes de la simulation
 */
coor_t* get_towns();

int* get_cutprefix();

/** initialisation du tableau des distances
 * \param seed valeur utilisé pour initialiser le générateur aléatoire de la map
 */
void genmap(long int seed);


#endif
