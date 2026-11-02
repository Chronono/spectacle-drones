/* tour.c, palier 5 : la tour dirige.
   Elle lit la figure, accueille les drones et donne à chacun son point.
   Chaque drone décide seul quand rentrer. L'affichage est dans radar.c.
   Usage : ./tour [figure.txt]          Ctrl+C : fermer la tour.        */

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "commun.h"
#include "tour.h"
#include "radar.h"

#define MAX_POINTS 128
#define MAX_DRONES 64

static Point figure[MAX_POINTS];
static int nb_points = 0;

static Drone *flotte = NULL;         /* la liste chaînée des drones connectés */

/* Un seul verrou protège tout ce que les threads partagent :
   la flotte, la figure et le journal. */
static pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;


/* ================= La figure, lue dans un fichier ================= */

/* figure.txt dessine le ciel en entier : un '#' par point. Rend le nombre de points. */
static int lire_figure(const char *nom)
{
    /* ✍ 1 À vous : ouvrir le fichier, FILE *f = fopen(nom, "r") ; NULL s'il
       n'existe pas : rendre 0. Puis, ligne y après ligne y (tant que y < HAUTEUR
       et que fgets(ligne, sizeof ligne, f) ne rend pas NULL), chaque caractère x
       (tant que x < LARGEUR et que ligne[x] n'est pas '\0') : un '#', c'est un
       point de plus, figure[nb_points].x = x, .y = y, nb_points++, sans dépasser
       MAX_POINTS. Fermer le fichier (fclose), et rendre nb_points. */
    return 0;
}


/* ================= La flotte, une liste chaînée ================= */

static void ajouter_a_la_flotte(Drone *d)
{
    d->suivant = flotte;
    flotte = d;
}

/* p pointe sur la flèche qui mène à d : le début de la liste, puis le
   champ suivant de chaque drone. On la fait pointer par-dessus d. */
static void retirer_de_la_flotte(Drone *d)
{
    Drone **p = &flotte;
    while (*p != d)
        p = &(*p)->suivant;
    *p = d->suivant;
}

/* Le drone rentre à la base : son point de la figure redevient libre. */
static void renvoyer_a_la_base(Drone *d)
{
    d->au_retour = 1;
    if (d->cible >= 0) {
        figure[d->cible].occupant = NULL;
        d->cible = -1;
    }
}


/* ================= Un thread par drone : la conversation ================= */

/* pthread_create impose cette forme : void *f(void *). Il ne connaît pas
   nos drones, il passe une adresse « de n'importe quoi » (void *), et
   attend une adresse en retour, que personne ne lira ici (NULL). */
static void *parler_au_drone(void *arg)
{
    Drone *d = arg;
    char ligne[64];
    const char *depart = "liaison perdue";

    /* le drone parle, la tour répond un ordre, et ainsi de suite */
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

        pthread_mutex_lock(&verrou);
        d->numero = numero;
        d->x = x;
        d->y = y;
        d->batterie = batterie;
        d->au_retour = rentre;
        /* ✍ 4 À vous : remplacer la ligne du dessus. S'il rentre et ne rentrait
           pas encore (rentre && !d->au_retour) : renvoyer_a_la_base(d), et
           noter(numero, "rentre à la base"). */

        /* ✍ 2 À vous : préparer l'ordre, char ordre[32] = "ATTENDS" ; s'il ne
           rentre pas et tient un point (!d->au_retour && d->cible >= 0) :
           sprintf(ordre, "CIBLE %d %d", ...), avec le x et le y de figure[d->cible]. */
        pthread_mutex_unlock(&verrou);

        /* ✍ 2 À vous : envoyer l'ordre au drone, une ligne, par dprintf. */
    }

    /* le drone est parti : on le retire de la flotte et on rend sa mémoire */
    pthread_mutex_lock(&verrou);
    /* ✍ 4 À vous : son point redevient libre, renvoyer_a_la_base(d). */
    retirer_de_la_flotte(d);
    noter(d->numero, depart);
    pthread_mutex_unlock(&verrou);

    close(d->sock);
    free(d);
    return NULL;
}


/* ================= L'accueil : un thread qui attend les drones ================= */

static int ouvrir_serveur(int port)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
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

static void *accueillir(void *arg)
{
    int serveur = *(int *)arg;
    while (1) {
        int s = accept(serveur, NULL, NULL);   /* attend qu'un drone appelle */
        if (s < 0)
            continue;

        Drone *d = calloc(1, sizeof *d);       /* calloc : tous les champs à 0 */
        d->sock = s;
        d->cible = -1;

        pthread_mutex_lock(&verrou);
        ajouter_a_la_flotte(d);
        pthread_mutex_unlock(&verrou);

        pthread_t t;
        pthread_create(&t, NULL, parler_au_drone, d);
        pthread_detach(t);                     /* personne n'attendra sa fin */
    }
    return NULL;
}


/* ================= Le chef d'orchestre : qui va où ================= */

/* Le drone libre le plus proche du point p, parmi ceux qui ont assez de
   batterie pour y aller puis rentrer. NULL s'il n'y en a pas. */
static Drone *plus_proche_libre(const Point *p)
{
    int retour = pas(p->x, p->y, BASE_X, BASE_Y) + MARGE_RETOUR;
    Drone *meilleur = NULL;
    int meilleure_distance = 0;

    for (Drone *d = flotte; d != NULL; d = d->suivant) {
        if (d->numero == 0 || d->au_retour || d->cible >= 0)
            continue;                          /* pas libre */
        int distance = pas(d->x, d->y, p->x, p->y);
        if (d->batterie - distance <= retour)
            continue;                          /* n'aurait pas de quoi rentrer */
        if (meilleur == NULL || distance < meilleure_distance) {
            meilleur = d;
            meilleure_distance = distance;
        }
    }
    return meilleur;
}

/* Chaque point libre prend le drone libre le plus proche. */
static void attribuer(void)
{
    for (int i = 0; i < nb_points; i++) {
        if (figure[i].occupant != NULL)
            continue;
        Drone *d = plus_proche_libre(&figure[i]);
        if (d != NULL) {
            d->cible = i;
            figure[i].occupant = d;
        }
    }
}

/* Combien de points ont leur drone, arrivé dessus. */
static int points_tenus(void)
{
    int tenus = 0;
    for (int i = 0; i < nb_points; i++) {
        Drone *d = figure[i].occupant;
        if (d != NULL && d->x == figure[i].x && d->y == figure[i].y)
            tenus++;
    }
    return tenus;
}


/* ================= Le programme ================= */

int main(int argc, char *argv[])
{
    const char *nom = argc > 1 ? argv[1] : "figure.txt";
    if (lire_figure(nom) == 0) {
        fprintf(stderr, "Pas de figure dans %s\n", nom);
        return 1;
    }

    signal(SIGPIPE, SIG_IGN);        /* un drone qui raccroche ne doit pas tuer la tour */

    int serveur = ouvrir_serveur(PORT_TOUR);
    if (serveur < 0) {
        fprintf(stderr, "Port %d déjà pris : une autre tour tourne ?\n", PORT_TOUR);
        return 1;
    }

    pthread_t accueil;
    pthread_create(&accueil, NULL, accueillir, &serveur);
    pthread_detach(accueil);

    ouvrir_radar();
    int reussi = 0;
    while (1) {                      /* jusqu'au Ctrl+C, qui tue la tour */
        pthread_mutex_lock(&verrou);
        attribuer();
        int tenus = points_tenus();
        if (tenus == nb_points && !reussi) {
            noter(0, "★ ECE complet !");
            reussi = 1;
        }
        dessiner(flotte, figure, nb_points, tenus);
        pthread_mutex_unlock(&verrou);
        usleep(TICK_MS * 1000);
    }
}
