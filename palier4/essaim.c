/* essaim.c, palier 4 : toute la flotte. Rien de neuf ici depuis le palier 3 :
   tout le travail du palier est dans tour.c.
   Usage : ./essaim [nombre de drones]      (32 par défaut)
   Ctrl+C        : rappel, tous les drones rentrent à la base.
   kill <pid>    : ce drone-là rentre à la base (un neuf le remplace).
   kill -9 <pid> : ce drone-là est abattu.                              */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include "commun.h"

#define MAX_DRONES 64

/* Pourquoi un drone s'arrête : son code de sortie, que l'essaim lira. */
enum { POSE = 0, A_PLAT = 1, PAS_DE_TOUR = 2 };


/* ================= Le drone : le code d'un processus fils ================= */

static volatile sig_atomic_t rappele = 0;

static void sur_sigterm(int sig)
{
    rappele = 1;                     /* on lève un drapeau, la boucle fera le reste */
}

/* Appelle la tour. Rend la socket, ou -1 si personne ne répond. */
static int se_connecter(void)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in tour = { 0 };
    tour.sin_family = AF_INET;
    tour.sin_port = htons(PORT_TOUR);
    tour.sin_addr.s_addr = htonl(INADDR_LOOPBACK);   /* 127.0.0.1 : cette machine */
    if (connect(s, (struct sockaddr *)&tour, sizeof tour) < 0)
        return -1;                                   /* personne au bout du fil */
    return s;
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

/* Le vol d'un drone : il décolle de la base, va en (cible_x, cible_y), y vole sur
   place, puis rentre. C'est le code du fils ; voler finit toujours par exit :
   le fils ne revient jamais dans main. */
static void voler(int numero, int cible_x, int cible_y)
{
    signal(SIGTERM, sur_sigterm);
    signal(SIGINT, SIG_IGN);         /* Ctrl+C est pour l'essaim, pas pour chaque drone */

    srand(getpid());                 /* chaque drone son hasard */
    int x = BASE_X;                  /* chaque drone décolle de la base */
    int y = BASE_Y;
    int batterie = 30 + rand() % 71; /* de 30 à 100 % */
    int retour = 0;                  /* 1 : il rentre à la base, pour de bon */
    int immobile = 0;

    int s = se_connecter();
    if (s < 0)
        exit(PAS_DE_TOUR);

    while (batterie > 0) {
        /* décider : rappelé, ou juste de quoi rejoindre la base ? on rentre */
        if (rappele || batterie <= pas(x, y, BASE_X, BASE_Y) + MARGE_RETOUR)
            retour = 1;

        /* dire à la tour où on est, et si on rentre */
        dprintf(s, "%s %d %d %d %d\n", retour ? "RENTRE" : "POS", numero, x, y, batterie);

        /* choisir où aller */
        if (retour) {
            if (x == BASE_X && y == BASE_Y) {
                dprintf(s, "POSE\n");
                exit(POSE);
            }
            cible_x = BASE_X;
            cible_y = BASE_Y;
        }

        /* avancer d'un pas, et payer en batterie */
        if (avancer(&x, &y, cible_x, cible_y))
            batterie--;              /* voler coûte 1 % par case */
        else if (++immobile % 10 == 0)
            batterie--;              /* le vol sur place aussi, plus lentement */

        usleep(TICK_MS * 1000);
    }

    dprintf(s, "CHUTE\n");
    exit(A_PLAT);                    /* exit ferme aussi la socket */
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
    case POSE:        return "posé à la base";
    case A_PLAT:      return "batterie à plat, chute";
    case PAS_DE_TOUR: return "pas de tour de contrôle (lancez ./tour d'abord)";
    default:          return "fin inconnue";
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
    signal(SIGINT, sur_sigint);

    pid_t pids[MAX_DRONES] = { 0 };  /* pids[i] : le processus du drone i + 1 */
    for (int i = 0; i < n; i++) {
        pids[i] = lancer_drone(i + 1);
        printf("drone %2d décolle (pid %d)\n", i + 1, pids[i]);
    }

    int en_vol = n;
    int rappel_fait = 0;
    while (en_vol > 0) {
        if (rappel_demande && !rappel_fait) {
            printf("\nRappel : tous les drones rentrent à la base.\n");
            for (int i = 0; i < n; i++)
                if (pids[i] > 0)
                    kill(pids[i], SIGTERM);
            rappel_fait = 1;
        }

        /* un drone s'est-il arrêté ? WNOHANG : on regarde sans attendre */
        int etat;
        pid_t p = waitpid(-1, &etat, WNOHANG);
        if (p <= 0) {
            usleep(100 * 1000);      /* personne : on revient voir dans 0,1 s */
            continue;
        }

        int numero = 0;
        for (int i = 0; i < n; i++)
            if (pids[i] == p) {
                numero = i + 1;
                pids[i] = 0;
                break;               /* trouvé : inutile de chercher plus loin */
            }
        en_vol--;

        if (WIFSIGNALED(etat))
            printf("💀 drone %2d abattu (signal %d)\n", numero, WTERMSIG(etat));
        else
            printf("   drone %2d : %s\n", numero, raison(WEXITSTATUS(etat)));

        /* posé à la base : un drone neuf prend la relève, sauf pendant le rappel */
        if (WIFEXITED(etat) && WEXITSTATUS(etat) == POSE && !rappel_fait) {
            pids[numero - 1] = lancer_drone(numero);
            en_vol++;
            printf("   drone %2d : relève, repart de la base (pid %d)\n",
                   numero, pids[numero - 1]);
        }
    }

    printf("Essaim : tout le monde est au sol.\n");
    return 0;
}
