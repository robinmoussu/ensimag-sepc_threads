#ifndef TSP_TSP_H
#define TSP_TSP_H

/** GLOBAL dernier minimum trouvé */
extern int minimum;

int present (int city, int hops, tsp_path_t path);

/** Travelling Salesman Problem
 *
 * \param hops     [in] Nombre de saut
 * \param len      [in] Distance parcouru par le voyageur
 * \param path     [in out] Chemin parcouru
 * \param *cuts    [in out] Nombre de coupure
 * \param sol      [in out] La meilleure solution entre `sol` avant l'appel et celle testée
 * \param *sol_len [out]
 *
 * \param GLOBAL minimun [in out] taille de la plus petite solution
 **/
void tsp (int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len);

#endif
