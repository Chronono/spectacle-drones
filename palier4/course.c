/* course.c : deux threads ajoutent 1 au même compteur, un million de fois chacun.
   Usage : ./course           sans verrou
           ./course verrou    avec verrou                                 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define TOURS 1000000

static long compteur = 0;            /* partagé : les deux threads le voient */
static int avec_verrou = 0;
static pthread_mutex_t verrou = PTHREAD_MUTEX_INITIALIZER;

static void *ajouter(void *arg)
{
    for (int i = 0; i < TOURS; i++) {
        if (avec_verrou)
            pthread_mutex_lock(&verrou);
        compteur++;                  /* lire, ajouter 1, écrire : trois temps */
        if (avec_verrou)
            pthread_mutex_unlock(&verrou);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    avec_verrou = argc > 1 && strcmp(argv[1], "verrou") == 0;

    pthread_t a, b;
    pthread_create(&a, NULL, ajouter, NULL);
    pthread_create(&b, NULL, ajouter, NULL);
    pthread_join(a, NULL);           /* attendre la fin de a, comme wait pour un fils */
    pthread_join(b, NULL);

    printf("%s : %ld, pour %d attendus\n",
           avec_verrou ? "avec verrou" : "sans verrou", compteur, 2 * TOURS);
    return 0;
}
