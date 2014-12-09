#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <assert.h>
#include <complex.h>

#include "tsp-types.h"
#include "tsp-genmap.h"

////////////////////////////////////////////////////////////////////////////////
// Variable static accessible d'une autre unitée de compilation

static int *cutprefix;

int* get_cutprefix()
{
    return cutprefix;
}

/** nombre de villes */
static int nb_towns=10;

int get_nb_towns()
{
    return nb_towns;
}

void set_nb_towns(int new_nb_towns)
{
    nb_towns = new_nb_towns;
}

/** Villes de la simulation */
static coor_t* towns;

coor_t* get_towns()
{
    return towns;
}

////////////////////////////////////////////////////////////////////////////////
// Variable static

static double barycentre_x=0, barycentre_y=0;
static double complex barycentre;

////////////////////////////////////////////////////////////////////////////////
// Fonction static

static int angle_barycentre(const void *a, const void *b);
static int trie_entier (const void *a, const void *b);

////////////////////////////////////////////////////////////////////////////////

static int angle_barycentre(const void *a, const void *b) {
    const coor_t *v1 = a;
    const coor_t *v2 = b;

    double complex c1 = v1->x + v1->y * I - barycentre;
    double complex c2 = v2->x + v2->y * I - barycentre;
    double a1 = carg(c1);
    double a2 = carg(c2);

    return (a1 - a2 > 0)?1:-1;
}

static int trie_entier (const void *a, const void *b) {
    const int *i1 = a;
    const int *i2 = b;

    return *i1 - *i2;
}

/** initialisation du tableau des distances */
void genmap () {

    int i, j;
    int dx, dy;

    if (nb_towns > MAX_TOWNS) {
        fprintf (stderr, "[Erreur] Trop de villes! Augmentez MAX_TOWNS dans tsp-types.h\n");
        exit (1);
    }
    towns = (coor_t*) calloc(nb_towns, sizeof(coor_t));

    srand48 ( get_myseed() ) ;

    for (i = 0; i < nb_towns; i++) {
        towns[i].x = lrand48 () % MAXX;
        towns[i].y = lrand48 () % MAXY;
        barycentre_x += towns[i].x;
        barycentre_y += towns[i].y;
    }
    barycentre_x /= nb_towns;
    barycentre_y /= nb_towns;
    barycentre = barycentre_x + barycentre_y * I;

    /* trier les villes en fonction de leurs angles par rapport au
     * barycentre */
    qsort(towns, nb_towns, sizeof(coor_t), angle_barycentre);

    /* sumprefix sur la distance minimale entre chaque ville */
    /* symetrie => ne regarder que les villes d'indice plus faible */
    cutprefix = (int *) calloc(nb_towns+1, sizeof(int));

    for (i = 0; i < nb_towns; i++) {
        cutprefix[i] = INT_MAX;
        for (j = 0; j < nb_towns; j++) {
            /* Un peu réaliste */
            dx = towns[i].x - towns[j].x;
            dy = towns[i].y - towns[j].y;
            set_distance(i, j, (int) sqrt ((double) ((dx * dx) + (dy * dy)))) ;
            if (i != j && get_distance(i, j) < cutprefix[i] && i < j )
                cutprefix[i] = get_distance(i, j);
        }
    }

    cutprefix[nb_towns] = 0;
    qsort(cutprefix, nb_towns+1, sizeof(int), trie_entier);
    long long int prefix=0;
    for(i=0; i < nb_towns+1; i++) {
        prefix += cutprefix[i];
        cutprefix[i] = prefix;
    }
}
