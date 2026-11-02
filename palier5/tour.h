#ifndef TOUR_H
#define TOUR_H

/* Les deux types de la tour, partagés avec radar.c qui les dessine. */

typedef struct Drone {
    int numero;              /* 0 tant qu'il n'a rien envoyé */
    int x, y, batterie;
    int cible;               /* le numéro du point de la figure qu'il tient, -1 sinon */
    int au_retour;           /* 1 : il rentre à la base */
    int sock;                /* la socket pour lui parler */
    struct Drone *suivant;   /* le drone suivant dans la flotte */
} Drone;

typedef struct {
    int x, y;
    Drone *occupant;         /* le drone qui tient ce point, NULL si personne */
} Point;

#endif
