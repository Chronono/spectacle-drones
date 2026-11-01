/* essaim.c, palier 1 : un drone, un processus.
   Usage : ./essaim [nombre de drones]      (32 par défaut)            */

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
           ✍ 4 À vous, ici, dans le fils : c'est le drone qui décide de rentrer, à chaque pas.
           S'il ne rentre pas encore (!retour) et que sa batterie ne dépasse pas
           pas(x, y, BASE_X, BASE_Y) + MARGE_RETOUR : retour = 1, et il le dit,
           "drone  3 : batterie 12 %, je rentre à la base". */

        /* choisir où aller
           ✍ 5 À vous : s'il rentre (retour), son cap devient la base : cible_x = BASE_X,
           cible_y = BASE_Y. S'il y est déjà (x == BASE_X et y == BASE_Y), il se pose : exit(POSE). */

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

static const char *raison(int code)
{
    switch (code) {
    case POSE:   return "posé à la base";
    case A_PLAT: return "batterie à plat, chute";
    default:     return "fin inconnue";
    }
}

/* Fait naître un drone : un fork. Rend son pid, au père.
   ✍ 1 À vous : créer cette fonction, ici, au-dessus de main.
       static pid_t lancer_drone(int numero)
   Elle contient, dans l'ordre :
     - la cible du drone, tirée au hasard dans le ciel, par le père, avant le fork :
       int cible_x = rand() % LARGEUR;   et   int cible_y = rand() % HAUTEUR;
     - fflush(stdout); : sinon le fils hérite du texte pas encore affiché ;
     - pid_t p = fork(); : un processus de plus, le drone ;
     - dans le fils (p == 0) : voler(numero, cible_x, cible_y); qui ne revient jamais ;
     - dans le père : return p; le pid du fils.
   Personne ne l'appelle encore : make le signale, "defined but not used". C'est le ✍ 2. */

int main(int argc, char *argv[])
{
    int n = argc > 1 ? atoi(argv[1]) : 32;
    if (n < 1 || n > MAX_DRONES) {
        fprintf(stderr, "Entre 1 et %d drones.\n", MAX_DRONES);
        return 1;
    }
    srand(time(NULL));

    /* faire naître les drones
       ✍ 2 À vous : n drones, donc n appels à lancer_drone. Une boucle
       for (int i = 0; i < n; i++), et dedans pid_t p = lancer_drone(i + 1); (numéros 1 à n).
       Après chaque appel, le père écrit "drone  3 décolle (pid 4242)" :
       printf("drone %2d décolle (pid %d)\n", i + 1, p); */

    /* attendre qu'ils s'arrêtent, un par un
       ✍ 3 À vous, dans le père : int etat; pid_t p; puis une boucle
       tant que p = wait(&etat) est > 0.
       wait rend le pid d'un fils qui s'est arrêté, ou -1 quand il n'en reste plus.
       Pour chacun : "   pid 4242 : posé à la base", par raison(WEXITSTATUS(etat)). */

    printf("Essaim : tout le monde est au sol.\n");
    return 0;
}
