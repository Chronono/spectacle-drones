# Bonus : la tour chez l'un, l'essaim chez l'autre

Le palier 5 est fini ? Jusqu'ici, la tour et l'essaim tournent dans le même Codespace : le drone appelle `127.0.0.1`, « cette machine ». Ce bonus les sépare : la tour dans le Codespace de l'un, l'essaim dans le Codespace de l'autre, et les lignes `POS` et `CIBLE` passent par Internet.

Ce qu'on y apprend : appeler une machine par son nom, et voir ce que le réseau coûte.

## À deux, ou seul

- **À deux** : **A** garde la tour, **B** lance l'essaim, chacun dans son Codespace. Puis vous échangez les rôles.
- **Seul** : tout dans votre Codespace, mais l'essaim appelle la tour par Internet. Ses lignes sortent de GitHub et reviennent.

Vous travaillez dans votre `palier5/`, et vous gardez le palier tel quel : sans argument en plus, `./essaim 32` doit toujours appeler `127.0.0.1`.

## Le problème : un Codespace ne reçoit pas d'appel

Un Codespace peut appeler n'importe qui sur Internet, mais personne ne peut l'appeler directement. Pour que B atteigne la tour de A, il faut un relais : **bore**, un petit programme qui ouvre un tunnel TCP.

- Chez A, `bore` appelle le serveur public `bore.pub` et lui dit : « ce qui arrive pour moi, renvoie-le sur mon port 5000 ».
- `bore.pub` répond par un numéro de port, tiré au hasard, par exemple `41234`.
- B appelle `bore.pub`, port `41234` : son appel arrive à la tour de A.

## A : ouvrir la tour

Premier terminal, comme d'habitude :

```bash
cd palier5
make
./tour
```

Deuxième terminal (le **+**), installer bore une fois, puis ouvrir le tunnel :

```bash
cd ~
curl -sL https://github.com/ekzhang/bore/releases/download/v0.6.0/bore-v0.6.0-x86_64-unknown-linux-musl.tar.gz | tar xz
./bore local 5000 --to bore.pub
```

bore écrit l'adresse publique de votre tour : `bore.pub:` suivi d'un numéro. Donnez ce numéro à B.

Tant que bore tourne, votre tour est ouverte à tout Internet. À la fin, Ctrl+C sur bore.

## B : appeler une machine par son nom

✍ **À vous**, dans `essaim.c` :

1. `main` lit deux arguments en plus : le nom de la machine de la tour, puis son port. `./essaim 32 bore.pub 41234`.
2. `se_connecter` appelle cette machine-là, et plus `INADDR_LOOPBACK`. Sans les deux arguments, rien ne change : `127.0.0.1` et `PORT_TOUR`.

`bore.pub` est un **nom**, pas une adresse : `connect` ne sait pas quoi en faire. C'est **`getaddrinfo`** qui transforme un nom (et un port) en adresse. Son manuel : `man 3 getaddrinfo`, et son exemple tout en bas.

<details>
<summary>Un coup de pouce</summary>

`getaddrinfo(nom, port, &indices, &resultat)` : le port s'écrit en texte (`"41234"`), et `indices` dit ce qu'on cherche (`AF_INET`, `SOCK_STREAM`), le reste à 0. Elle rend 0 si elle a trouvé. `resultat->ai_addr` et `resultat->ai_addrlen` vont directement à `connect`. À la fin, `freeaddrinfo(resultat)`.

</details>

Puis :

```bash
make
./essaim 32 bore.pub 41234      # le numéro que A vous a donné
```

**Seul** : même chose, dans un troisième terminal de votre Codespace.

## Regarder

- Où s'affiche le radar ? Que voit B ?
- B fait Ctrl+C : que voit A ?
- A ferme sa tour (Ctrl+C) en plein vol : que deviennent les drones de B ?
- Le spectacle va-t-il à la même vitesse qu'avant ? Mettez `TICK_MS` à 400, puis à 0 (dans `commun.h`, chez B), et comparez. Pourquoi ? (Relisez `voler` : que fait un drone entre deux pas ?)

## Encore plus loin

✍ **Pour A** : à chaque drone qui appelle, la tour écrit au journal l'adresse d'où il appelle. `accept` peut la remplir : `man 2 accept`, et `inet_ntop` pour l'écrire en texte.

Lancez, et regardez l'adresse qui s'affiche quand B appelle par bore. Est-ce celle que vous attendiez ? Pourquoi ?
