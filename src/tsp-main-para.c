﻿#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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


////////////////////////////////////////////////////////////////////////////////
// macro

/** macro de mesure de temps, retourne une valeur en nanosecondes */
#define TIME_DIFF(t1, t2) \
    ((t2.tv_sec - t1.tv_sec) * 1000000000ll + (long long int) (t2.tv_nsec - t1.tv_nsec))


////////////////////////////////////////////////////////////////////////////////
// Variable static accessible d'une autre unitée de compilation

/** tableau des distances */
static int distance [MAX_TOWNS] [MAX_TOWNS] = {};

int get_distance(int x, int y)
{
    return distance[x][y];
}

void set_distance(int x, int y, int new_distance)
{
    distance[x][y] = new_distance;
}

/** graine */
static long int myseed= 0;

long int get_myseed()
{
    return myseed;
}

////////////////////////////////////////////////////////////////////////////////

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

/** Écrit dans la sortie d'erreur une courte description du programme
 * \param name [in] nom du programme
 */
static void usage(const char *name);

////////////////////////////////////////////////////////////////////////////////

static void generate_tsp_jobs (struct tsp_queue *q, int hops, int len,
            tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len,
            int depth)
{
    if (len >= get_minimum()) {
        (*cuts)++ ;
        return;
    }

    if (hops == depth) {
        /* On enregistre du travail à faire plus tard... */
        add_job (q, path, hops, len);
    } else {
        int me = path [hops - 1];
        for (int i = 0; i < get_nb_towns(); i++) {
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

    set_nb_towns(atoi(argv[optind]));
    myseed = atol(argv[optind+1]);
    nb_threads = atoi(argv[optind+2]);
    assert(get_nb_towns() > 0);
    assert(nb_threads > 0);

    /* generer la carte et la matrice de distance */
    fprintf (stderr, "ncities = %3d\n", get_nb_towns());
    genmap ();

    init_queue (&q);

    clock_gettime (CLOCK_REALTIME, &t1);

    memset (path, -1, MAX_TOWNS * sizeof (int));
    path[0] = 0;

    /* mettre les travaux dans la file d'attente */
    generate_tsp_jobs (&q, 1, 0, path, &cuts, sol, & sol_len, 3);
    no_more_jobs (&q);

    /* calculer chacun des travaux */
    tsp_path_t solution;
    memset (solution, -1, MAX_TOWNS * sizeof (int));
    solution[0] = 0;
    while (!empty_queue (&q)) {
        int hops = 0, len = 0;
        get_job (&q, solution, &hops, &len);
        tsp (hops, len, solution, &cuts, sol, &sol_len);
    }

    clock_gettime (CLOCK_REALTIME, &t2);

    if (affiche_sol)
        print_solution_svg (sol, sol_len);

    perf = TIME_DIFF (t1,t2);
    printf("<!-- # = %d seed = %ld len = %d threads = %d time = %lld.%03lld ms ( %lld coupures ) -->\n",
            get_nb_towns(), myseed, sol_len, nb_threads,
            perf/1000000ll, perf%1000000ll, cuts);

    return 0 ;
}
