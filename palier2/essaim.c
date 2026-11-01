/* essaim.c, palier 2 : abattre, rappeler, remplacer.
   Usage : ./essaim [nombre de drones]      (32 par défaut)
   Le but, à la fin du palier :
   Ctrl+C        : rappel, tous les drones rentrent à la base.
   kill <pid>    : ce drone-là rentre à la base (un neuf le remplace).
   kill -9 <pid> : ce drone-là est abattu.                              */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include "essaim.h"

#define MAX_DRONES 64

/* Comment un drone finit : son code de sortie, que l'essaim lira.
   Posé à la base, ou batterie à plat. La tour en ajoutera une, au palier 3. */
enum { POSE = 0, A_PLAT = 1 };


/* ================= Le drone : le code d'un processus fils ================= */

/* Un drapeau, levé par le traitant, lu par la boucle du drone.
   volatile : relu en mémoire à chaque fois ; sig_atomic_t : écrit d'un seul coup. */
static volatile sig_atomic_t rappele = 0;

static void sur_sigterm(int sig)
{
    rappele = 1;                     /* on lève un drapeau, la boucle fera le reste */
}

/* Un pas vers (cible_x, cible_y), en diagonale si besoin. Rend 1 s'il a bougé. */
static int avancer(int *x, int *y, int cible_x, int cible_y)
{
    int dx = (cible_x > *x) - (cible_x < *x); /* +1, 0 ou -1 */
    int dy = (cible_y > *y) - (cible_y < *y);
    *x += dx;
    *y += dy;
    return dx != 0 || dy != 0;
}

/* Le nombre de pas de (x1, y1) à (x2, y2). Un pas bouge x et y à la fois, pour 1 % :
   il en faut autant que le plus grand des deux écarts. De (12, 3) à (29, 11) :
   8 pas en diagonale, puis 9 tout droit, 17 pas. */
static int pas(int x1, int y1, int x2, int y2)
{
    int dx = abs(x1 - x2);
    int dy = abs(y1 - y2);
    return dx > dy ? dx : dy;
}

/* Le vol d'un drone : il décolle de la base, va en (cible_x, cible_y), y vole sur
   place, puis rentre. C'est le code du fils ; voler finit toujours par exit :
   le fils ne revient jamais dans main. */
static void voler(int numero, int cible_x, int cible_y)
{
    /* ✍ 2 À vous : SIGTERM, c'est le rappel de ce drone : signal(SIGTERM, sur_sigterm);
       ✍ 3 À vous : Ctrl+C est pour l'essaim, pas pour chaque drone : signal(SIGINT, SIG_IGN); */

    srand(getpid());                 /* chaque drone son hasard */
    int x = BASE_X;                  /* chaque drone décolle de la base */
    int y = BASE_Y;
    int batterie = 30 + rand() % 71; /* de 30 à 100 % */
    int retour = 0;                  /* 1 : il rentre à la base, pour de bon */
    int immobile = 0;

    printf("drone %2d : je suis le pid %d, batterie %d %%, cap sur (%d, %d)\n",
           numero, getpid(), batterie, cible_x, cible_y);

    while (batterie > 0) {
        /* décider : juste de quoi rejoindre la base ? on rentre
           ✍ 2 À vous : rappelé (rappele vaut 1), il rentre aussi. Et il dit pourquoi :
           "drone  2 : rappelé, je rentre à la base", ou "batterie faible". */
        if (!retour && batterie <= pas(x, y, BASE_X, BASE_Y) + MARGE_RETOUR) {
            retour = 1;
            printf("drone %2d : batterie %d %%, je rentre à la base\n", numero, batterie);
        }

        /* choisir où aller */
        if (retour) {
            if (x == BASE_X && y == BASE_Y)
                exit(POSE);
            cible_x = BASE_X;
            cible_y = BASE_Y;
        }

        /* avancer d'un pas, et payer en batterie */
        if (avancer(&x, &y, cible_x, cible_y)) {
            batterie--;              /* voler coûte 1 % par case */
            if (!retour && x == cible_x && y == cible_y)
                printf("drone %2d : arrivé en (%d, %d), batterie %d %%\n", numero, x, y, batterie);
        } else if (++immobile % 10 == 0) {
            batterie--;              /* le vol sur place aussi, plus lentement */
        }

        usleep(TICK_MS * 1000);
    }

    exit(A_PLAT);
}


/* ================= L'essaim : le processus père ================= */

static volatile sig_atomic_t rappel_demande = 0;

static void sur_sigint(int sig)
{
    rappel_demande = 1;
}

static const char *raison(int code)
{
    switch (code) {
    case POSE:   return "posé à la base";
    case A_PLAT: return "batterie à plat, chute";
    default:     return "fin inconnue";
    }
}

/* Fait naître un drone : un fork. Rend son pid. */
static pid_t lancer_drone(int numero)
{
    int cible_x = rand() % LARGEUR;  /* le père choisit la cible avant le fork : */
    int cible_y = rand() % HAUTEUR;  /* le fils en reçoit une copie */

    fflush(stdout);                  /* sinon le fils hérite du texte pas encore affiché */
    pid_t p = fork();
    if (p == 0)
        voler(numero, cible_x, cible_y);
    return p;
}

int main(int argc, char *argv[])
{
    int n = argc > 1 ? atoi(argv[1]) : 32;
    if (n < 1 || n > MAX_DRONES) {
        fprintf(stderr, "Entre 1 et %d drones.\n", MAX_DRONES);
        return 1;
    }
    srand(time(NULL));
    /* ✍ 3 À vous : Ctrl+C, c'est le rappel général : signal(SIGINT, sur_sigint);
       avant les fork. */

    pid_t pids[MAX_DRONES] = { 0 };  /* pids[i] : le processus du drone i + 1 */
    for (int i = 0; i < n; i++) {
        pids[i] = lancer_drone(i + 1);
        printf("drone %2d décolle (pid %d)\n", i + 1, pids[i]);
    }

    int rappel_fait = 0;             /* 1 : le rappel est parti (✍ 3) ; la relève s'arrête */

    /* ✍ 4 À vous : wait dort tant qu'aucun drone ne s'arrête ; pendant ce temps, personne
       ne regarde rappel_demande. Remplacer cette boucle par while (en_vol > 0), avec
       int en_vol = n; avant :
         - p = waitpid(-1, &etat, WNOHANG) : 0 si personne ne s'est arrêté ;
           alors usleep(100 * 1000), et continue ;
         - un drone s'est arrêté : en_vol-- ; un drone repart (la relève) : en_vol++. */
    int etat;
    pid_t p;
    while ((p = wait(&etat)) > 0) {
        /* ✍ 3 À vous : rappel demandé (rappel_demande), et pas encore fait (!rappel_fait) :
           "Rappel : tous les drones rentrent à la base.", kill(pids[i], SIGTERM) à chaque
           drone en vol (pids[i] > 0), puis rappel_fait = 1. */

        int numero = 0;              /* le numéro du drone, retrouvé par son pid */
        for (int i = 0; i < n; i++)
            if (pids[i] == p) {
                numero = i + 1;
                pids[i] = 0;
                break;               /* trouvé : inutile de chercher plus loin */
            }

        /* ✍ 1 À vous : abattu par un signal ? Si WIFSIGNALED(etat), WTERMSIG(etat) donne
           le signal : "💀 drone  3 abattu (signal 9)". Sinon, la ligne ci-dessous. */
        printf("   drone %2d : %s\n", numero, raison(WEXITSTATUS(etat)));

        /* Donné : la relève. Posé à la base (WIFEXITED, et le code vaut POSE), hors rappel :
           un drone neuf part de la base, sous le même numéro. */
        if (WIFEXITED(etat) && WEXITSTATUS(etat) == POSE && !rappel_fait) {
            pids[numero - 1] = lancer_drone(numero);
            printf("   drone %2d : relève, repart de la base (pid %d)\n",
                   numero, pids[numero - 1]);
        }
    }

    printf("Essaim : tout le monde est au sol.\n");
    return 0;
}
