#ifndef _TSP_JOB_H
#define _TSP_JOB_H
/** gestion des files de jobs */

/** Structure pour la tête de file */
struct tsp_queue {
    struct tsp_cell *first;
    struct tsp_cell *last;
    int end;
};

/** Initialise la file [q]. */
extern void init_queue (struct tsp_queue *q);

/** Ajoute un job à la file [q]: chemin [p], [hops] villes parcourues, longueur parcourue [len]. */
extern void add_job (struct tsp_queue *q, tsp_path_t p, int hops, int len) ;

/** Enlève un job de la file [q], le stocke dans [p], [hops] et [len].
 *
 * \param q [in out] La file dont le premier élément est retiré
 * \param p [out] Reçoie la copie du chemin de q->first
 * \param hops [out] Reçoie la copie du nombre de saut de q->first
 * \param len [out] Reçoie la copie de la longueur du chemin de q->first
 *
 * \return 0 si la file est vide.
 **/
extern int get_job (struct tsp_queue *q, tsp_path_t p, int *hops, int *len) ;

/** Enregistre qu'il n'y aura plus de jobs ajoutés à la file. */
extern void no_more_jobs (struct tsp_queue *q) ;

/** Retourne 1 si la file est vide (i.e. no_more_jobs() a été appelé, et tous
 * les jobs ont été enlevés), 0 sinon. */
extern int empty_queue (struct tsp_queue *q);

#endif /* _TSP_JOBS */
