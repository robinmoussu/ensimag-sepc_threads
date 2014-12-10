#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>

#include "tsp-types.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"

////////////////////////////////////////////////////////////////////////////////
// Variable static accessible d'une autre unitée de compilation

/** dernier minimum trouvé */
static int minimum = INT_MAX;

// attention la fonction tsp modifie le minimum

int get_minimum() {
    return minimum;
}

////////////////////////////////////////////////////////////////////////////////

static pthread_mutex_t mutex_minimum = PTHREAD_MUTEX_INITIALIZER;

int present (int city, int hops, tsp_path_t path)
{
    int ret = 0;
    pthread_mutex_lock(&mutex_minimum);
    for (int i = 0; i < hops; i++) {
        if (path [i] == city) {
            ret = 1;
            break;
        }
    }
    pthread_mutex_unlock(&mutex_minimum);

    return ret ;
}



void tsp (int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len)
{
    if (len + get_cutprefix(get_nb_towns()-hops) >= minimum) {

        pthread_mutex_lock(&mutex_minimum);
        (*cuts)++ ;
        pthread_mutex_unlock(&mutex_minimum);

        return;
    }

    if (hops == get_nb_towns()) {

        pthread_mutex_lock(&mutex_minimum);
        int me = path [hops - 1];
        int dist = get_distance(me, 0); // retourner en 0
        if ( len + dist < minimum ) {
            minimum = len + dist;
            *sol_len = len + dist;
            memcpy(sol, path, get_nb_towns()*sizeof(int));
            print_solution (path, len+dist);
        }
        pthread_mutex_unlock(&mutex_minimum);

    } else {
        int me = path [hops - 1];
        for (int i = 0; i < get_nb_towns(); i++) {
            if (!present (i, hops, path)) {

                pthread_mutex_lock(&mutex_minimum);
                path[hops] = i;
                int dist = get_distance(me, i);
                pthread_mutex_unlock(&mutex_minimum);

                tsp (hops + 1, len + dist, path, cuts, sol, sol_len);
            }
        }
    }
}

