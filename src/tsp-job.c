/** \file
 *
 * \brief implémentation des queues de jobs, nul besoin de lire dans un premier temps
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "tsp-types.h"
#include "tsp-job.h"

struct tsp_job {
    tsp_path_t path;
    int hops;
    int len;
};

struct tsp_cell {
    struct tsp_job tsp_job;
    struct tsp_cell *next;
};

////////////////////////////////////////////////////////////////////////////////

static pthread_mutex_t mutex_jobs     = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cond_consumer  = PTHREAD_COND_INITIALIZER;

////////////////////////////////////////////////////////////////////////////////

void init_queue (struct tsp_queue *q) {
    pthread_mutex_lock (& mutex_jobs);

    q->first = 0;
    q->last = 0;
    q->end = 0;

    pthread_mutex_unlock (& mutex_jobs);
}

int empty_queue (struct tsp_queue *q) {
    int ret;

    pthread_mutex_lock (& mutex_jobs);

    ret = (q->first == 0) && (q->end == 1);

    pthread_mutex_unlock (& mutex_jobs);

    return ret;
}

void add_job (struct tsp_queue *q, tsp_path_t p, int hops, int len) {
    struct tsp_cell *ptr;

    pthread_mutex_lock (& mutex_jobs);

    ptr = malloc (sizeof (*ptr));
    if (!ptr) {
        printf ("L'allocation a echoue, recursion infinie ?\n");
        exit (1);
    }
    ptr->next = 0;
    ptr->tsp_job.len = len;
    ptr->tsp_job.hops = hops;
    memcpy (ptr->tsp_job.path, p, hops * sizeof(p[0]));

    if (q->first == 0) {
        q->first = q->last = ptr;
    } else {
        q->last->next = ptr;
        q->last = ptr;
    }

    pthread_cond_signal (& cond_consumer);
    pthread_mutex_unlock (& mutex_jobs);
}

int get_job (struct tsp_queue *q, tsp_path_t p, int *hops, int *len) {
    struct tsp_cell *ptr;

    pthread_mutex_lock (& mutex_jobs);

    while (q->first == 0) {
        pthread_cond_wait (& cond_consumer, & mutex_jobs);
    }

    ptr = q->first;

    q->first = ptr->next;
    if (q->first == 0) {
        q->last = 0;
    }

    *len = ptr->tsp_job.len;
    *hops = ptr->tsp_job.hops;
    memcpy (p, ptr->tsp_job.path, *hops * sizeof(p[0]));

    free (ptr);

    pthread_mutex_unlock (& mutex_jobs);
    return 1;
}

void no_more_jobs (struct tsp_queue *q) {
    pthread_mutex_lock (& mutex_jobs);

    q->end = 1;

    pthread_mutex_unlock (& mutex_jobs);
}











