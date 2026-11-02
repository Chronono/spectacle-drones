# Le spectacle de drones

C embarqué, ING4, ECE. Deux séances de Linux : Linux 1 (paliers 1 à 3) et Linux 2 (paliers 4 et 5).

Trente-deux drones décollent de leur base. Une tour de contrôle leur donne à chacun un point, et ils écrivent ECE dans le ciel. Chaque drone est un **processus**, on leur parle par **signaux**, la tour leur parle par **sockets**, et écoute toute la flotte à la fois grâce aux **threads**.

## Ouvrir votre Codespace

1. Connectez-vous à GitHub.
2. Sur cette page : **Code**, onglet **Codespaces**, **Create codespace on main**.
3. Deux minutes plus tard, VS Code s'ouvre dans le navigateur, avec un terminal en bas.

Le Codespace est à vous : vos fichiers y restent d'une séance à l'autre. Pour le retrouver : https://github.com/codespaces. Il s'éteint seul après 30 minutes sans activité ; il se rallume quand vous le rouvrez.

## Un palier

Un dossier par palier. Le départ d'un palier est le corrigé du précédent : en retard, vous repartez à niveau.

```bash
cd palier1          # entrer dans le dossier du palier
make                # compiler
./essaim 3          # lancer trois drones
```

Les `✍ À vous` du code disent quoi écrire. Les avertissements de `make` au départ montrent ce qui n'est pas encore utilisé : c'est votre travail.

| Commande | Ce qu'elle fait |
| --- | --- |
| `cd palier2` | entrer dans un dossier ; `cd ..` pour remonter |
| `ls` | lister les fichiers du dossier |
| `make` | compiler ce qui a changé |
| `./essaim 3` | lancer le programme `essaim` de ce dossier, avec l'argument `3` |
| Ctrl+C | arrêter le programme qui tourne dans ce terminal |
| `ps -C essaim -o pid,ppid,stat,cmd` | voir les processus `essaim` |
| `kill 4242` | envoyer SIGTERM au processus 4242 |
| `man 2 fork` | le manuel de `fork` ; `q` pour sortir |

Un deuxième terminal : le **+** en haut à droite du panneau des terminaux.

Un drone fait un pas toutes les 40 ms (`TICK_MS`, dans `essaim.h` puis `commun.h`) : un vol dure de quelques secondes à une demi-minute. Pour le regarder voler pas à pas, mettez 400, puis `make`.

## Ce que voit votre prof

Chaque `make`, chaque `./essaim` et chaque `./tour` laisse une ligne, envoyée à votre prof : votre pseudo GitHub, l'heure, le dossier, la commande, si elle a réussi (ou le signal qui l'a arrêtée), sa durée, et pour `make` le nombre d'erreurs et d'avertissements, avec la première erreur. Pas votre code, ni le reste du terminal. C'est pour voir où ça coince.

Vos lignes sont dans `~/.traces-drones/` ; le script, dans `.outils/trace.sh`.

## Linux 2

Les paliers 4 et 5 sont dans ce dépôt. Dans votre Codespace, un terminal à la racine :

```bash
git pull
bash .outils/installer-trace.sh
```

puis un nouveau terminal (le **+**). `git pull` ajoute `palier4/` et `palier5/` à côté des vôtres, sans toucher à vos fichiers.

Deux terminaux : `./tour` dans l'un, `./essaim` dans l'autre (32 drones). Au palier 4, `./course` montre deux threads qui se disputent un compteur ; `./course verrou`, la même chose avec un verrou.
