#ifndef ESSAIM_H
#define ESSAIM_H

/* Les réglages de l'essaim : le ciel, la base, la vitesse, la marge.
   Au palier 3, ils passent dans commun.h, que la tour partage. */

#define LARGEUR          30    /* le ciel, en cases */
#define HAUTEUR          12
#define BASE_X   (LARGEUR - 1) /* la base, en bas à droite */
#define BASE_Y   (HAUTEUR - 1)
#define TICK_MS         400    /* un pas toutes les 400 ms */
#define MARGE_RETOUR      3    /* un drone rentre quand il n'a plus que de quoi
                                  rejoindre la base, plus 3 % (1 % par case) */

#endif
