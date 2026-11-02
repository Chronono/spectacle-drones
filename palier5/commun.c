/* commun.c : le code que la tour et l'essaim partagent. Il arrive au palier 3 :
   pas() y déménage depuis essaim.c (la tour s'en sert au palier 5),
   et lire_ligne() lit le protocole. */

#include <stdlib.h>
#include <unistd.h>
#include "commun.h"

/* Le nombre de pas de (x1, y1) à (x2, y2). Un pas bouge x et y à la fois, pour 1 % :
   il en faut autant que le plus grand des deux écarts. De (12, 3) à (29, 11) :
   8 pas en diagonale, puis 9 tout droit, 17 pas. */
int pas(int x1, int y1, int x2, int y2)
{
    int dx = abs(x1 - x2);
    int dy = abs(y1 - y2);
    return dx > dy ? dx : dy;
}

/* TCP envoie un flux d'octets, pas des messages : un read peut rendre une
   demi-ligne ou deux lignes d'un coup. On lit donc octet par octet
   jusqu'au '\n'. */
int lire_ligne(int sock, char *ligne, int taille)
{
    int n = 0;
    while (n < taille - 1) {
        char c;
        int lu = read(sock, &c, 1);
        if (lu <= 0)
            return lu;             /* 0 : raccroché ; -1 : erreur */
        if (c == '\n')
            break;
        ligne[n++] = c;
    }
    ligne[n] = '\0';
    return n;
}
