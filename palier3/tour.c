/* tour.c, palier 3 : la tour voit un drone.
   Elle attend un appel, écoute ce drone jusqu'à ce qu'il raccroche,
   puis prend l'appel suivant. L'affichage est dans radar.c.
   Usage : ./tour          Ctrl+C : fermer la tour.                     */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "commun.h"
#include "tour.h"
#include "radar.h"

/* Ouvre le standard : une socket qui attend les appels sur ce port.
   Rend la socket, ou -1 si le port est déjà pris. */
static int ouvrir_serveur(int port)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);    /* une prise, pour TCP sur IPv4 */
    int oui = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &oui, sizeof oui);   /* relancer sans attendre */

    struct sockaddr_in adresse = { 0 };
    adresse.sin_family = AF_INET;                  /* IPv4 */
    adresse.sin_port = htons(port);                /* octets dans l'ordre du réseau */
    adresse.sin_addr.s_addr = htonl(INADDR_ANY);   /* toutes les adresses de la machine */

    if (bind(s, (struct sockaddr *)&adresse, sizeof adresse) < 0)
        return -1;                                 /* le port est déjà pris */
    listen(s, 64);                                 /* 64 appels en attente, au plus */
    return s;
}

int main(void)
{
    int serveur = ouvrir_serveur(PORT_TOUR);
    if (serveur < 0) {
        fprintf(stderr, "Port %d déjà pris : une autre tour tourne ?\n", PORT_TOUR);
        return 1;
    }

    ouvrir_radar();
    dessiner(NULL, NULL, 0, 0);      /* un ciel vide, en attendant le premier appel */
    while (1) {                      /* jusqu'au Ctrl+C, qui tue la tour */
        /* Donné : décrocher. accept dort jusqu'à ce qu'un drone appelle, et rend une
           socket neuve, rien que pour lui parler ; serveur reste au standard. */
        int s = accept(serveur, NULL, NULL);

        Drone d = { 0 };             /* un seul drone, rangé sur la pile */
        d.cible = -1;                /* pas de point de figure aujourd'hui */
        char ligne[64];

        /* ✍ 2 À vous : écouter ce drone. Tant que lire_ligne(s, ligne, sizeof ligne) rend
           plus de 0 : si sscanf(ligne, "POS %d %d %d %d", ...) rend 4 (dans &d.numero, &d.x,
           &d.y, &d.batterie) : d.au_retour = 0 ; si "RENTRE %d %d %d %d" rend 4 :
           d.au_retour = 1 ; puis dessiner(&d, NULL, 0, 0) : un drone seul, pas de figure.
           ✍ 3 À vous : une autre ligne (POSE, CHUTE) : noter(d.numero, ligne), au journal.
           ✍ 4 À vous : lire_ligne a rendu 0, il a raccroché : noter(d.numero, "a raccroché"),
           close(s), puis dessiner(NULL, NULL, 0, 0) : un ciel vide. */
    }
}
