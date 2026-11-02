#ifndef RADAR_H
#define RADAR_H

#include "tour.h"

/* Le radar : tout l'affichage de la tour. Fourni tout fait. */

void ouvrir_radar(void);     /* efface l'écran */

/* Ajoute une ligne au journal (les quatre derniers événements).
   numero 0 : pas de « drone N : » devant. */
void noter(int numero, const char *texte);

/* Dessine le ciel, les drones, les batteries les plus basses et le journal.
   flotte : le premier drone d'une liste chaînée (ou un drone seul, ou NULL).
   figure, nb_points : les points de la figure (NULL, 0 s'il n'y en a pas).
   tenus : combien de points ont leur drone. */
void dessiner(Drone *flotte, Point figure[], int nb_points, int tenus);

#endif
