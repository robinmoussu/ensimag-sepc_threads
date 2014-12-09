﻿#include <assert.h>
#include <string.h>
#include <limits.h>

#include "tsp-types.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"

////////////////////////////////////////////////////////////////////////////////
// Variable static accessible d'une autre unitée de compilation

/** dernier minimum trouvé */
static int minimum = INT_MAX;


int get_minimum() {
    return minimum;
}

////////////////////////////////////////////////////////////////////////////////

/** résolution du problème du voyageur de commerce */
int present (int city, int hops, tsp_path_t path)
{
    for (int i = 0; i < hops; i++) {
        if (path [i] == city) {
            return 1;
        }
    }

    return 0 ;
}



void tsp (int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len)
{
    if (len + get_cutprefix()[(get_nb_towns()-hops)] >= minimum) {
        (*cuts)++ ;
        return;
    }

    if (hops == get_nb_towns()) {
        int me = path [hops - 1];
        int dist = get_distance(me, 0); // retourner en 0
        if ( len + dist < minimum ) {
            minimum = len + dist;
            *sol_len = len + dist;
            memcpy(sol, path, get_nb_towns()*sizeof(int));
            print_solution (path, len+dist);
        }
    } else {
        int me = path [hops - 1];
        for (int i = 0; i < get_nb_towns(); i++) {
            if (!present (i, hops, path)) {
                path[hops] = i;
                int dist = get_distance(me, i);
                tsp (hops + 1, len + dist, path, cuts, sol, sol_len);
            }
        }
    }
}

