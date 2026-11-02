#ifndef COMMUN_H
#define COMMUN_H

/* Ce que la tour et les drones doivent savoir tous les deux. */

#define PORT_TOUR      5000    /* la tour écoute sur ce port */
#define LARGEUR          30    /* le ciel, en cases : figure.txt le dessine en entier */
#define HAUTEUR          12
#define BASE_X   (LARGEUR - 1) /* la base, en bas à droite */
#define BASE_Y   (HAUTEUR - 1)
#define TICK_MS         40     /* un pas toutes les 40 ms ; 400 pour le voir voler pas à pas */
#define MARGE_RETOUR      3    /* un drone rentre quand il n'a plus que de quoi
                                  rejoindre la base, plus 3 % (1 % par case) */

/* Les messages, une ligne de texte chacun :
     drone -> tour : "POS <numero> <x> <y> <batterie>"     voilà où je suis
                     "RENTRE <numero> <x> <y> <batterie>"  pareil, et je rentre à la base
                     "POSE"                                je suis posé à la base
                     "CHUTE"                               batterie à plat
     tour -> drone : "CIBLE <x> <y>"                       va là
                     "ATTENDS"                             reste où tu es        */

/* Le nombre de pas de (x1, y1) à (x2, y2). */
int pas(int x1, int y1, int x2, int y2);

/* Lit une ligne sur la socket, sans le '\n'.
   Rend sa longueur, 0 si l'autre a raccroché, -1 en cas d'erreur. */
int lire_ligne(int sock, char *ligne, int taille);

#endif
