---
title: Jeu de la vie — Guide d'utilisation
tags: [poo, cpp, projet]
---

# Jeu de la vie

Implémentation C++17 du jeu de la vie de John Conway, avec un mode console et un mode graphique SFML.

## Environnement et compilation

Sous Ubuntu, installer les outils nécessaires :

```bash
sudo apt-get install g++ make libsfml-dev fonts-dejavu-core
```

La police système **DejaVu Sans** est nécessaire au mode graphique ; elle est fournie par le paquet `fonts-dejavu-core`.

Puis, depuis ce dossier :

```bash
make
```

L'exécutable produit est `jeu_de_la_vie`.

## Fichier d'entrée

La première ligne contient le nombre de lignes puis le nombre de colonnes. Les lignes suivantes contiennent uniquement des `0` (mort) et des `1` (vivant). Le fichier `exemple.txt` reprend l'exemple du sujet.

## Mode console

```bash
./jeu_de_la_vie exemple.txt console 10
```

Le dernier argument est le nombre maximal d'itérations ; il vaut 100 par défaut. Les fichiers `iteration_0.txt`, `iteration_1.txt`, etc. sont écrits dans `exemple.txt_out`, dans le dossier courant. L'itération 0 conserve l'état initial. Avant d'écrire un nouveau run, le mode console supprime uniquement les fichiers nommés exactement `iteration_<chiffres>.txt` (par exemple `iteration_3.txt`) dans ce dossier ; les autres fichiers et dossiers sont conservés. Le programme s'arrête dès qu'une itération ne modifie plus aucune cellule, même si la limite n'est pas atteinte.

## Mode graphique

```bash
./jeu_de_la_vie exemple.txt graphique 100 500
```

Les arguments sont le délai initial entre deux itérations en millisecondes et le nombre maximal d'itérations. Leurs valeurs par défaut sont 100 ms et 1000 itérations. Le délai initial doit être au moins 10 ms ; une valeur inférieure est refusée au lancement. Les raccourcis `+` et `-` bornent ensuite le délai entre 10 et 2000 ms.

La barre latérale permet de contrôler la simulation et de choisir un outil d'édition :

| Action | Raccourci |
|---|---|
| Lecture / Pause | `Espace` |
| Pas à pas | `N` ou `flèche droite` |
| Vitesse `-` / `+` | `-` / `+` |
| Réinitialiser la grille initiale (en pause) | `R` |
| Effacer toutes les cellules | `C` |
| Sélectionner l'outil Cellule | `0` |
| Sélectionner Planeur / Clignotant / Bloc / Crapaud | `1` / `2` / `3` / `4` |

`+` réduit le délai de 50 ms (minimum 10 ms) et `-` l'augmente de 50 ms (maximum 2000 ms). Avec l'outil **Cellule**, un clic gauche sur une case inverse son état et mémorise le nouvel état. En maintenant le bouton gauche et en glissant, ce même état est appliqué aux nouvelles cases traversées, sans jamais les rebasculer ; relâcher arrête le dessin et sortir de la grille n'applique rien. Avec un motif sélectionné, le clic place le motif en le centrant sur la case ; le placement est refusé sans modification si le motif ne tient pas entièrement dans la grille, et le message `Motif hors limites` apparaît alors dans la barre latérale. Ce message est effacé par la prochaine action réussie, un changement d'outil, un reset ou un clear. Toute édition met la simulation en pause.

Lorsque la limite d'itérations est atteinte, une édition réussie efface l'état de limite et accorde un nouveau budget d'itérations égal au budget initial, à partir de l'itération courante ; le compteur d'itérations n'est pas remis à zéro. Reset et Clear restaurent le budget initial. La répétition automatique des touches est désactivée : maintenir une touche ne la répète pas. `R` restaure la grille initiale en pause ; le lancement du mode graphique reste automatique. Le crapaud prédéfini est une forme 2x4.

Les cellules vivantes sont blanches et les cellules mortes noires. L'état, l'itération, le délai, l'outil courant et l'éventuel message de placement sont affichés dans la barre latérale. Lorsque la grille devient stable ou que la limite d'itérations est atteinte, la simulation se met en pause mais la fenêtre reste ouverte. Fermer la fenêtre quitte le programme.

## Tests unitaires intégrés

```bash
make test
```

Ou directement :

```bash
./jeu_de_la_vie --test
```

`Tests::verifierGrille` reçoit une grille initiale, une grille attendue et un nombre arbitraire d'itérations ; il s'arrête dès que la stabilité est détectée. Les scénarios métier vérifient un clignotant après une et deux itérations, ainsi que le scénario `le bloc reste stable lors d'une demande de cinq itérations`. Les tests de fichiers et de console sont regroupés dans `TestsFichiers::executer`, appelé par `Tests::executer` : ils couvrent les allers-retours lecture/écriture, les cas d'erreur de fichier, le mode console, le nettoyage ciblé des anciens `iteration_N.txt` et la préservation des autres fichiers. Le programme renvoie 0 si tous les tests réussissent, sinon 1.

## Organisation du code

- `Jeu` contrôle les itérations, détecte la stabilité et gère les éditions, l'effacement et la réinitialisation.
- `Grille` compte les voisines et applique simultanément les nouveaux états.
- `Cellule` possède un `EtatCellule` polymorphe : `EtatVivant` ou `EtatMort`.
- `Motif` représente une forme rectangulaire et fournit les quatre motifs prédéfinis.
- `RegleConway` applique les règles via l'interface abstraite `Regle`, indépendante des états.
- `FichierGrille` isole la lecture et l'écriture des fichiers.
- `ModeConsole` et `ModeGraphique` utilisent le modèle sans modifier ses règles ; le mode graphique s'appuie sur SFML et propose l'édition interactive.
- `Tests` vérifie le modèle et appelle `TestsFichiers`, qui isole les tests de fichiers et de mode console.

Les diagrammes demandés se trouvent dans [CONCEPTION.md](CONCEPTION.md). Les extensions optionnelles de grille torique, d'obstacles et de parallélisation ne font pas partie de cette version.
