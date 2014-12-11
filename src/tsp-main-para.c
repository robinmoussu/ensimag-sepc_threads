#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <assert.h>
#include <complex.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "tsp-types.h"
#include "tsp-job.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"


/* macro de mesure de temps, retourne une valeur en nanosecondes */
#define TIME_DIFF(t1, t2) \
  ((t2.tv_sec - t1.tv_sec) * 1000000000ll + (long long int) (t2.tv_nsec - t1.tv_nsec))


/* tableau des distances */
tsp_distance_matrix_t distance ={};

/** Param?tres **/

/* nombre de villes */
int nb_towns=10;
/* graine */
long int myseed= 0;
/* nombre de threads */
int nb_threads=1;

/* affichage SVG */
bool affiche_sol= false;

static pthread_mutex_t mutex_cuts = PTHREAD_MUTEX_INITIALIZER;

static void generate_tsp_jobs (struct tsp_queue *q, int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len, int depth)
{
    if (len >= minimum) {
        (*cuts)++ ;
        return;
    }
    
    if (hops == depth) {
        /* On enregistre du travail ? faire plus tard... */
        add_job (q, path, hops, len);
    } else {
        int me = path [hops - 1];        
        for (int i = 0; i < nb_towns; i++) {
            if (!present (i, hops, path)) {
                path[hops] = i;
                int dist = distance[me][i];
                generate_tsp_jobs (q, hops + 1, len + dist, path, cuts, sol, sol_len, depth);
            }
        }
    }
}

static void usage(const char *name) {
  fprintf (stderr, "Usage: %s [-s] <ncities> <seed> <nthreads>\n", name);
  exit (-1);
}

typedef struct {
    tsp_path_t *path;
    struct tsp_queue *q;
    long long int *cuts;
    tsp_path_t *sol;
    int *sol_len;
} args_create_t;

void * create_tsp_job_parallele(void *args)
{
    args_create_t *a = args;
    long long int cuts = 0; // On utilise une variable cuts par threads, et on reporte le total seulement à la fin

    fprintf(stderr, "creation de thread\n");

    memset (*(a->path), -1, MAX_TOWNS * sizeof (int));
    *(a->path)[0] = 0;

    /* mettre les travaux dans la file d'attente */
    generate_tsp_jobs (a->q, 1, 0, *(a->path), &cuts, *(a->sol), a->sol_len, 3);
    no_more_jobs (a->q);

    pthread_mutex_lock(&mutex_cuts);
    *(a->cuts) += cuts;
    pthread_mutex_unlock(&mutex_cuts);

    fprintf(stderr, "fin de thread\n");
    return 0;
}

typedef struct {
    struct tsp_queue *q;
    long long int *cuts;
    tsp_path_t *sol;
    int *sol_len;
} args_consumme_t;

void* consumme_tsp_job_parallele(void *args)
{
    args_consumme_t* a = args;
    long long int cuts = 0; // On utilise une variable cuts par threads, et on reporte le total seulement à la fin

    fprintf(stderr, "creation de thread\n");

    /* calculer chacun des travaux */
    tsp_path_t solution;
    memset (solution, -1, MAX_TOWNS * sizeof (int));
    solution[0] = 0;
    while (!empty_queue (a->q)) {
        int hops = 0, len = 0;

        get_job (a->q, solution, &hops, &len);
        tsp (hops, len, solution, &cuts, *(a->sol), a->sol_len);

    }

    // On met à jour le cuts global
    pthread_mutex_lock(&mutex_cuts);
    *(a->cuts) += cuts;
    pthread_mutex_unlock(&mutex_cuts);

    fprintf(stderr, "fin de thread\n");
    return 0;
}

int main (int argc, char **argv)
{
    unsigned long long perf;
    tsp_path_t path;
    tsp_path_t sol;
    int sol_len;
    long long int cuts = 0;
    struct tsp_queue q;
    struct timespec t1, t2;

    /* lire les arguments */
    int opt;
    while ((opt = getopt(argc, argv, "s")) != -1) {
      switch (opt) {
      case 's':
	affiche_sol = true;
	break;
      default:
	usage(argv[0]);
	break;
      }
    }

    if (optind != argc-3)
      usage(argv[0]);

    nb_towns = atoi(argv[optind]);
    myseed = atol(argv[optind+1]);
    nb_threads = atoi(argv[optind+2]);
    assert(nb_towns > 0);
    assert(nb_threads > 0);
   
    minimum = INT_MAX;
      
    /* generer la carte et la matrice de distance */
    fprintf (stderr, "ncities = %3d\n", nb_towns);
    genmap ();

    init_queue (&q);

    pthread_t *threads = calloc(nb_threads, sizeof(*threads));
    int thread_creation = 0;

    clock_gettime (CLOCK_REALTIME, &t1);
    args_create_t args_create = {
        &path,
        &q,
        &cuts,
        &sol,
        &sol_len,
    };
    // Le premier thread est réservé pour la creation des jobs
    pthread_create( threads + thread_creation, NULL, create_tsp_job_parallele, &args_create);
    
    args_consumme_t args_consumme = {
        &q,
        &cuts,
        &sol,
        &sol_len
    };
    // Le premier thread est réservé pour la creation des jobs
    for (int i = 1; i < nb_threads; i++) {
        pthread_create( threads + i, NULL, consumme_tsp_job_parallele,
                &args_consumme);
    }

    // On transforme le thread de creation en thread de consommation quand il a fini de produire
    pthread_join(threads[thread_creation], NULL);
    pthread_create( threads + thread_creation, NULL, consumme_tsp_job_parallele, &args_consumme);

    // On attend la fine du programme
    for (int i = 0; i < nb_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime (CLOCK_REALTIME, &t2);
    free(threads);

    if (affiche_sol)
      print_solution_svg (sol, sol_len);

    perf = TIME_DIFF (t1,t2);
    printf("<!-- # = %d seed = %ld len = %d threads = %d time = %lld.%03lld ms ( %lld coupures ) -->\n",
	   nb_towns, myseed, sol_len, nb_threads,
	   perf/1000000ll, perf%1000000ll, cuts);

    return 0 ;
}
