/* radar.c : tout l'affichage de la tour, dans le terminal. Fourni tout fait. */

#include <stdio.h>
#include <stdlib.h>
#include "commun.h"
#include "radar.h"

#define MAX_DRONES 64

/* Les codes ANSI : des suites de caractères que le terminal prend pour des
   ordres, pas pour du texte à afficher. Ils commencent tous par \033[ (Échap). */
#define EFFACER_ECRAN      "\033[2J"
#define CURSEUR_EN_HAUT    "\033[H"
#define EFFACER_FIN_LIGNE  "\033[K"

#define VERT    "\033[32m"
#define JAUNE   "\033[33m"
#define ROUGE   "\033[31m"
#define CYAN    "\033[36m"
#define GRAS    "\033[1m"
#define PALE    "\033[2m"
#define NORMAL  "\033[0m"            /* annule couleur, gras et pâle */

#define BATTERIE_BONNE    50         /* au-dessus : vert */
#define BATTERIE_FAIBLE   25         /* jusque-là : rouge ; entre les deux : jaune */


/* ================= Le journal ================= */

static char journal[4][80];          /* les quatre derniers événements */
static int nb_evenements = 0;

void noter(int numero, const char *texte)
{
    char *ligne = journal[nb_evenements % 4];   /* on écrase le plus ancien */
    if (numero > 0)
        snprintf(ligne, sizeof journal[0], "drone %d : %s", numero, texte);
    else
        snprintf(ligne, sizeof journal[0], "%s", texte);
    nb_evenements++;
}


/* ================= Les morceaux du dessin ================= */

static const char *couleur(int batterie)
{
    if (batterie > BATTERIE_BONNE)
        return VERT;
    if (batterie > BATTERIE_FAIBLE)
        return JAUNE;
    return ROUGE;
}

/* Chaque ligne finit ainsi : on efface ce qui restait de l'image d'avant. */
static void finir_ligne(void)
{
    printf(EFFACER_FIN_LIGNE "\n");
}

/* Le haut ou le bas du cadre : ┌────┐ ou └────┘ */
static void afficher_bord(const char *coin_gauche, const char *coin_droit)
{
    printf(" %s", coin_gauche);
    for (int x = 0; x < LARGEUR; x++)
        printf("──");
    printf("%s", coin_droit);
}

/* Une case du ciel, sur deux caractères. */
static void afficher_case(const Drone *d, int x, int y, int est_un_point)
{
    if (d != NULL) {
        if (est_un_point)
            printf(GRAS);            /* un drone sur un point de la figure brille */
        printf("%s", couleur(d->batterie));
        printf("● " NORMAL);
    } else if (x == BASE_X && y == BASE_Y) {
        printf("◆ ");
    } else if (est_un_point) {
        printf(PALE "· " NORMAL);    /* un point de la figure qui attend son drone */
    } else {
        printf("  ");
    }
}

/* Une ligne du tableau de droite : « drone 12   18 %  ↘ base » */
static void afficher_batterie(const Drone *d)
{
    printf("   drone %2d  ", d->numero);
    printf("%s%3d %%" NORMAL, couleur(d->batterie), d->batterie);
    if (d->au_retour)
        printf("  ↘ base");
}

/* Pour qsort : a et b sont les adresses de deux cases du tableau,
   et chaque case contient un Drone * (d'où les deux étoiles). */
static int plus_faible_d_abord(const void *a, const void *b)
{
    const Drone *da = *(Drone *const *)a;
    const Drone *db = *(Drone *const *)b;
    return da->batterie - db->batterie;
}


/* ================= Le dessin entier ================= */

void ouvrir_radar(void)
{
    printf(EFFACER_ECRAN);
}

void dessiner(Drone *flotte, Point figure[], int nb_points, int tenus)
{
    /* 1. où est chaque drone, où sont les points de la figure */
    Drone *ciel[HAUTEUR][LARGEUR] = { { NULL } };
    char est_un_point[HAUTEUR][LARGEUR] = { { 0 } };
    Drone *tous[MAX_DRONES];
    int n = 0;

    for (int i = 0; i < nb_points; i++)
        est_un_point[figure[i].y][figure[i].x] = 1;

    for (Drone *d = flotte; d != NULL; d = d->suivant) {
        if (d->numero == 0)
            continue;                /* pas encore de position */
        if (d->x < 0 || d->x >= LARGEUR || d->y < 0 || d->y >= HAUTEUR)
            continue;                /* hors du ciel : on ne le dessine pas */
        ciel[d->y][d->x] = d;
        if (n < MAX_DRONES)
            tous[n++] = d;
    }

    /* 2. le tableau de droite : les batteries les plus basses d'abord */
    qsort(tous, n, sizeof tous[0], plus_faible_d_abord);

    /* 3. le titre */
    printf(CURSEUR_EN_HAUT);
    printf(GRAS " TOUR DE CONTRÔLE" NORMAL);
    if (nb_points > 0)
        printf("   figure %d/%d", tenus, nb_points);
    printf("   drones en vol %d", n);
    if (nb_points > 0 && tenus == nb_points)
        printf("   " GRAS CYAN "★ ECE ★" NORMAL);
    finir_ligne();

    /* 4. le ciel, ligne par ligne, avec une batterie au bout de chaque ligne */
    afficher_bord("┌", "┐");
    printf("   Batteries les plus basses");
    finir_ligne();

    for (int y = 0; y < HAUTEUR; y++) {
        printf(" │");
        for (int x = 0; x < LARGEUR; x++)
            afficher_case(ciel[y][x], x, y, est_un_point[y][x]);
        printf("│");

        if (y < n)
            afficher_batterie(tous[y]);
        finir_ligne();
    }

    afficher_bord("└", "┘");
    finir_ligne();

    /* 5. les quatre derniers événements */
    for (int i = 0; i < 4; i++) {
        int k = nb_evenements - 4 + i;
        if (k >= 0)
            printf(" %s", journal[k % 4]);
        finir_ligne();
    }

    fflush(stdout);                  /* tout part à l'écran d'un coup */
}
