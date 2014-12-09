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

#include "tsp-types.h"
#include "tsp-job.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"


/** macro de mesure de temps, retourne une valeur en nanosecondes */
#define TIME_DIFF(t1, t2) \
    ((t2.tv_sec - t1.tv_sec) * 1000000000ll + (long long int) (t2.tv_nsec - t1.tv_nsec))


// variable globales

/** tableau des distances */
tsp_distance_matrix_t distance ={};

/** nombre de villes */
int nb_towns=10;
/** graine */
long int myseed= 0;

////////////////////////////////////////////////////////////////////////////////

typedef struct {
    struct tsp_queue *q;
    int hops;
    int len;
    path_elem_t *path;
    long long int *cuts;
    path_elem_t *sol;
    int *sol_len;
    int depth;
} args_generate_tsp_t;

typedef struct {
    struct tsp_queue *q;
    path_elem_t* solution;
    long long int *cuts;
    path_elem_t *sol;
    int *sol_len;
} args_consume_tsp_t;

////////////////////////////////////////////////////////////////////////////////

static void* generate_tsp_jobs_paralel(void *args);
static void* consume_tsp_jobs_parallele(void *args);

/**
 * struct tsp_queue *q
 * param hops Nombre de saut
 * param len
 * param path
 * param long int *cuts
 * param sol
 * param *sol_len
 * param depth
 */
static void generate_tsp_jobs (struct tsp_queue *q, int hops, int len,
            tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len,
            int depth);

static void usage(const char *name);

////////////////////////////////////////////////////////////////////////////////

static void* generate_tsp_jobs_paralel(void *args)
{
    // on récupère les arguments
    args_generate_tsp_t *args_generate_tsp = (args_generate_tsp_t*) args;

    // on appele la fonction réelle
    generate_tsp_jobs(
        args_generate_tsp->q,
        args_generate_tsp->hops,
        args_generate_tsp->len,
        args_generate_tsp->path,
        args_generate_tsp->cuts,
        args_generate_tsp->sol,
        args_generate_tsp->sol_len,
        args_generate_tsp->depth
        );
    no_more_jobs(args_generate_tsp->q);

    return 0;
}

static void generate_tsp_jobs (struct tsp_queue *q, int hops, int len,
            tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len,
            int depth)
{
    if (len >= minimum) {
        (*cuts)++ ;
        return;
    }

    if (hops == depth) {
        /* On enregistre du travail à faire plus tard... */
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

static void* consume_tsp_jobs_parallele(void *args)
{
    int hops = 0, len = 0;

    // on récupère les arguments
    args_consume_tsp_t *args_consume_tsp = (args_consume_tsp_t*) args;

    get_job(args_consume_tsp->q,
            args_consume_tsp->solution,
            &hops,
            &len
        );
    tsp(    hops,
            len,
            args_consume_tsp->solution,
            args_consume_tsp->cuts,
            args_consume_tsp->sol,
            args_consume_tsp->sol_len
        );

    return 0;
}

static void usage(const char *name) {
    fprintf (stderr, "Usage: %s [-s] <ncities> <seed> <nthreads>\n", name);
    exit (-1);
}

////////////////////////////////////////////////////////////////////////////////

int main (int argc, char **argv)
{
    unsigned long long perf;
    tsp_path_t path;
    tsp_path_t sol;
    int sol_len;
    long long int cuts = 0;
    struct tsp_queue q;
    struct timespec t1, t2;

    /** nombre de threads */
    int nb_threads=1;

    /** affichage SVG */
    bool affiche_sol= false;

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

    clock_gettime (CLOCK_REALTIME, &t1);

    memset (path, -1, MAX_TOWNS * sizeof (int));
    path[0] = 0;

    /* mettre les travaux dans la file d'attente */
    args_generate_tsp_t args_generate_tsp;

    args_generate_tsp.q       = &q;
    args_generate_tsp.hops    = 1;
    args_generate_tsp.len     = 0;
    args_generate_tsp.path    = path;
    args_generate_tsp.cuts    = &cuts;
    args_generate_tsp.sol     = sol;
    args_generate_tsp.sol_len = & sol_len;
    args_generate_tsp.depth   = 3;

    generate_tsp_jobs_paralel(&args_generate_tsp);

    /* calculer chacun des travaux */
    tsp_path_t solution;
    memset (solution, -1, MAX_TOWNS * sizeof (int));
    solution[0] = 0;

    args_consume_tsp_t args_consume_tsp;

    args_consume_tsp.q        = &q;
    args_consume_tsp.solution = solution;
    args_consume_tsp.cuts     = &cuts;
    args_consume_tsp.sol      = sol;
    args_consume_tsp.sol_len  = &sol_len;

    while (!empty_queue (&q)) {
        consume_tsp_jobs_parallele(&args_consume_tsp);
    }

    clock_gettime (CLOCK_REALTIME, &t2);

    if (affiche_sol)
        print_solution_svg (sol, sol_len);

    perf = TIME_DIFF (t1,t2);
    printf("<!-- # = %d seed = %ld len = %d threads = %d time = %lld.%03lld ms ( %lld coupures ) -->\n",
            nb_towns, myseed, sol_len, nb_threads,
            perf/1000000ll, perf%1000000ll, cuts);

    return 0 ;
}
