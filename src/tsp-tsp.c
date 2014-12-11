#include <assert.h>
#include <string.h>
#include <pthread.h>

#include "tsp-types.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"

/* dernier minimum trouv? */
int minimum;

/* r?solution du probl?me du voyageur de commerce */
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
  static pthread_mutex_t mutex_sol = PTHREAD_MUTEX_INITIALIZER;

  if (len + cutprefix[(nb_towns-hops)] >= minimum) {
      (*cuts)++ ; // étrangement protéger cette variable rend le temps d'exécution extremement lent
      return;
    }
    
    if (hops == nb_towns) {
	    int me = path [hops - 1];
	    int dist = distance[me][0]; // retourner en 0
        if ( len + dist < minimum ) {
		    minimum = len + dist;

            pthread_mutex_lock(&mutex_sol);
		    *sol_len = minimum;
		    memcpy(sol, path, nb_towns*sizeof(int));
            pthread_mutex_unlock(&mutex_sol);

		    print_solution (path, len+dist);
	    }
    } else {
        int me = path [hops - 1];        
        for (int i = 0; i < nb_towns; i++) {
            if (!present (i, hops, path)) {
                path[hops] = i;
                int dist = distance[me][i];
                tsp (hops + 1, len + dist, path, cuts, sol, sol_len);
            }
        }
    }
}

