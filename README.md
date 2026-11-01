# Le spectacle de drones

C embarqué, ING4, ECE. Séances 7 et 9 : Linux.

Trente-deux drones décollent de leur base. Une tour de contrôle leur donne à chacun un point, et ils écrivent ECE dans le ciel. Chaque drone est un **processus**, on leur parle par **signaux**, la tour leur parle par **sockets**.

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

## Séance 9

Les paliers 4 et 5 arriveront dans ce dépôt. Dans votre Codespace, un terminal à la racine :

```bash
git pull
```
