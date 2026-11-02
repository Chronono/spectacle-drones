/* tour.c, palier 4 : toute la flotte.
   Un thread accueille les drones ; chaque drone a son thread, qui l'écoute.
   Les drones vivent dans une liste chaînée, la flotte, que tous les threads
   partagent sous un seul verrou. L'affichage est dans radar.c.
   Usage : ./tour          Ctrl+C : fermer la tour.                     */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "commun.h"
#include "tour.h"
#include "radar.h"

#define MAX_DRONES 64

static Drone *flotte = NULL;         /* la liste chaînée des drones connectés */

/* Un seul verrou protège tout ce que les threads partagent :
   la flotte et le journal. */
static pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;


/* ================= La flotte, une liste chaînée ================= */

/* Le nouveau drone passe en tête : il pointe sur l'ancienne tête. */
static void ajouter_a_la_flotte(Drone *d)
{
    /* ✍ 2 À vous : deux lignes. Le suivant de d devient la tête actuelle
       (flotte), puis la tête devient d. */
}

/* Donné : p pointe sur la flèche qui mène à d : le début de la liste, puis
   le champ suivant de chaque drone. On la fait pointer par-dessus d. */
static void retirer_de_la_flotte(Drone *d)
{
    Drone **p = &flotte;
    while (*p != d)
        p = &(*p)->suivant;
    *p = d->suivant;
}


/* ================= Un thread par drone : la conversation ================= */

/* pthread_create impose cette forme : void *f(void *). Il ne connaît pas
   nos drones, il passe une adresse « de n'importe quoi » (void *), et
   attend une adresse en retour, que personne ne lira ici (NULL). */
static void *parler_au_drone(void *arg)
{
    Drone *d = arg;                  /* void * -> Drone * : on sait ce qu'on a passé */
    char ligne[64];
    const char *depart = "liaison perdue";

    /* Donné : la conversation du palier 3, avec un drone rangé sur le tas (d->). */
    while (lire_ligne(d->sock, ligne, sizeof ligne) > 0) {
        int numero, x, y, batterie;
        int rentre;

        if (sscanf(ligne, "POS %d %d %d %d", &numero, &x, &y, &batterie) == 4)
            rentre = 0;
        else if (sscanf(ligne, "RENTRE %d %d %d %d", &numero, &x, &y, &batterie) == 4)
            rentre = 1;
        else if (strcmp(ligne, "POSE") == 0) {
            depart = "posé à la base";
            break;
        } else if (strcmp(ligne, "CHUTE") == 0) {
            depart = "batterie à plat, chute";
            break;
        } else
            continue;

        /* ✍ 4 À vous : ces cinq lignes touchent la flotte : sous le verrou. */
        d->numero = numero;
        d->x = x;
        d->y = y;
        d->batterie = batterie;
        d->au_retour = rentre;
    }

    /* ✍ 3 À vous : le drone est parti. Le retirer de la flotte
       (retirer_de_la_flotte), noter(d->numero, depart) au journal,
       raccrocher (close), puis rendre sa mémoire (free).
       ✍ 4 À vous : le retrait et le journal, sous le verrou. */

    return NULL;
}


/* ================= L'accueil : un thread qui attend les drones ================= */

/* Donné : ouvre le standard, une socket qui attend les appels sur ce port.
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
    listen(s, MAX_DRONES);
    return s;
}

/* Le thread d'accueil : décroche chaque appel, range le drone dans la
   flotte et lui donne son thread. arg : l'adresse de la socket du standard. */
static void *accueillir(void *arg)
{
    int serveur = *(int *)arg;
    while (1) {
        int s = accept(serveur, NULL, NULL);   /* attend qu'un drone appelle */
        if (s < 0)
            continue;

        noter(0, "un drone appelle");

        /* ✍ 2 À vous : un drone sur le tas, Drone *d = calloc(1, sizeof *d) :
           tous ses champs à 0. Sa socket : s ; sa cible : -1. Le ranger dans la
           flotte (ajouter_a_la_flotte), puis lui donner son thread :
           pthread_create(&t, NULL, parler_au_drone, d), et pthread_detach(t).
           ✍ 4 À vous : le journal et l'ajout, sous le verrou. */
    }
    return NULL;
}


/* ================= Le programme ================= */

int main(void)
{
    int serveur = ouvrir_serveur(PORT_TOUR);
    if (serveur < 0) {
        fprintf(stderr, "Port %d déjà pris : une autre tour tourne ?\n", PORT_TOUR);
        return 1;
    }

    /* ✍ 1 À vous : lancer le thread d'accueil. pthread_t accueil; puis
       pthread_create(&accueil, NULL, accueillir, &serveur) : accueillir
       reçoit l'adresse de serveur. Puis pthread_detach(accueil). */

    ouvrir_radar();
    while (1) {                      /* jusqu'au Ctrl+C, qui tue la tour */
        /* ✍ 4 À vous : le dessin lit toute la flotte : sous le verrou. */
        dessiner(flotte, NULL, 0, 0);
        usleep(TICK_MS * 1000);
    }
}
