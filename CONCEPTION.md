---
title: Jeu de la vie — Conception UML
tags: [poo, uml, projet]
---

# Conception UML

## Diagramme de classes

```mermaid
classDiagram
    class Jeu {
        -grille_ : Grille
        -initiale_ : Grille
        -regle_ : const Regle&
        -iteration_ : int
        -stable_ : bool
        +iterer() void
        +iteration() int
        +estStable() bool
        +grille() const Grille&
        +basculerCellule(ligne, colonne) void
        +modifierCellule(ligne, colonne, vivante) void
        +effacer() void
        +reinitialiser() void
        +placerMotif(motif, ligneCentre, colonneCentre) bool
    }
    class Grille {
        -lignes_ : int
        -colonnes_ : int
        -cellules_ : vector
        -compterVoisinesVivantes(ligne, colonne) int
        +lignes() int
        +colonnes() int
        +estVivante(ligne, colonne) bool
        +modifierCellule(ligne, colonne, vivante) void
        +basculerCellule(ligne, colonne) void
        +effacer() void
        +evoluer(regle) bool
        +aMemesEtats(autre) bool
    }
    class Motif {
        -nom_ : string
        -cellules_ : vector
        +Motif(nom, cellules)
        +nom() string
        +lignes() int
        +colonnes() int
        +estVivante(ligne, colonne) bool
        +predefinis()$ vector
    }
    class Cellule {
        -etat_ : EtatCellule
        +estVivante() bool
        +changerEtat(vivante) void
    }
    class EtatCellule {
        <<abstract>>
        +estVivante() bool
        +cloner() EtatCellule
    }
    class EtatVivant {
        +estVivante() bool
        +cloner() EtatCellule
    }
    class EtatMort {
        +estVivante() bool
        +cloner() EtatCellule
    }
    class Regle {
        <<abstract>>
        +doitEtreVivante(vivante, voisines) bool
    }
    class RegleConway {
        +doitEtreVivante(vivante, voisines) bool
    }
    class FichierGrille {
        +lire(chemin)$ Grille
        +ecrire(grille, chemin)$ void
    }
    class ModeConsole {
        +executer(jeu, chemin, iterations)$ void
    }
    class ModeGraphique {
        -delaiMs_ : int
        -motifs_ : vector
        -police_ : sf::Font
        -boutons_ : vector
        -motifSelectionne_ : int
        -nombreIterationsMaximum_ : int
        -budgetInitial_ : int
        -enCours_ : bool
        -limiteAtteinte_ : bool
        -glisserActif_ : bool
        -etatGlisser_ : bool
        -derniereLigneGlisser_ : int
        -derniereColonneGlisser_ : int
        -message_ : string
        -statutY_ : float
        -hauteurInterface_ : float
        +ModeGraphique(delaiMs)
        +executer(jeu, iterationsMaximum) void
        -creerInterface(largeurGrille) void
        -ajouterBouton(libelle, commande, indiceMotif, x, largeur, y) void
        -afficher(fenetre, jeu, tailleCellule, largeurGrille) void
        -traiterTouche(touche, jeu) void
        -traiterClicBouton(position, jeu) bool
        -traiterClicGrille(position, jeu, tailleCellule) void
        -traiterGlisser(position, jeu, tailleCellule) void
        -prolongerBudgetSiLimite(jeu) void
        -basculerLecture(jeu) void
        -avancer(jeu) void
        -reinitialiser(jeu) void
        -effacer(jeu) void
        -changerDelai(variation) void
        -selectionnerCellule() void
        -selectionnerMotif(indice) void
    }
    class Tests {
        -verifierGrille(initiale, attendue, iterations)$ bool
        +executer()$ int
    }
    class TestsFichiers {
        +executer()$ bool
    }

    Jeu "1" *-- "2" Grille : courante et initiale
    Jeu "1" o-- "1" Regle : utilise
    Jeu ..> Motif : place
    Grille "1" *-- "1..*" Cellule : contient
    Cellule "1" *-- "1" EtatCellule : possède
    EtatCellule <|-- EtatVivant
    EtatCellule <|-- EtatMort
    Regle <|-- RegleConway
    Grille ..> Regle : utilise
    FichierGrille ..> Grille : lit et écrit
    ModeConsole ..> Jeu : pilote
    ModeGraphique ..> Jeu : pilote et édite
    ModeGraphique "1" *-- "0..*" Motif : propose
    Tests ..> Jeu : vérifie
    Tests ..> TestsFichiers : appelle
    TestsFichiers ..> FichierGrille : teste
    TestsFichiers ..> ModeConsole : teste
```

Notes de cohérence :

- `Jeu::regle_` est une référence non propriétaire `const Regle&` ; la règle doit vivre au moins aussi longtemps que le `Jeu`.
- `ModeGraphique` mémorise le budget initial. Une édition réussie après la limite efface `limiteAtteinte_` et fixe `nombreIterationsMaximum_` à `iteration + budgetInitial_`, sans remettre le compteur à zéro.
- `R` réinitialise en pause ; Reset et Clear remettent `nombreIterationsMaximum_` à `budgetInitial_`.
- Le glisser gauche réutilise `etatGlisser_` et `Jeu::modifierCellule` ; le message `Motif hors limites` est porté par `message_`.
- La répétition automatique du clavier est désactivée sur la fenêtre SFML.
- `Motif::predefinis` fournit Planeur 3x3, Clignotant 3x3, Bloc 2x2 et Crapaud 2x4.
- `ModeConsole::executer` supprime au préalable, dans le dossier de sortie, uniquement les fichiers réguliers nommés exactement `iteration_<chiffres>.txt` ; les autres fichiers et dossiers sont conservés.

## Diagramme d'activité

```mermaid
flowchart TD
    A([Début]) --> B[Lire les paramètres et le fichier]
    B --> C{Entrée valide ?}
    C -- Non --> D[Afficher l'erreur]
    D --> Z([Fin])
    C -- Oui --> E[Créer la grille, les cellules et le jeu]
    E --> F{Mode choisi}
    F -- Console --> G[Créer le dossier, nettoyer les anciens iteration_N.txt ciblés et écrire l'itération 0]
    G --> I{Itérations restantes et grille non stable ?}
    I -- Non --> Z
    I -- Oui --> J[Compter les voisines et calculer tous les états futurs]
    J --> K[Appliquer simultanément les états et détecter la stabilité]
    K --> M[Écrire la grille dans son fichier]
    M --> I

    F -- Graphique --> H[Ouvrir la fenêtre et créer la barre latérale]
    H --> N{Fenêtre ouverte ?}
    N -- Non --> Z
    N -- Oui --> O[Traiter lecture, pause, pas à pas, vitesse, reset en pause et édition clic/glisser]
    O --> P{Édition de cellule, glisser, effacement ou motif valide ?}
    P -- Édition valide --> Q[Modifier la grille, mettre en pause et prolonger le budget si la limite était atteinte]
    P -- Motif refusé --> W[Afficher Motif hors limites]
    P -- Aucune édition --> R{Lecture active, délai écoulé, non stable et limite non atteinte ?}
    Q --> R
    W --> R
    R -- Non --> V[Afficher la grille et l'interface]
    R -- Oui --> S[Calculer et appliquer une itération]
    S --> T{Stable ou limite atteinte ?}
    T -- Non --> V
    T -- Oui --> U[Mettre en pause sans fermer la fenêtre]
    U --> V
    V --> N
```

## Diagramme de séquence — une itération en mode console

```mermaid
sequenceDiagram
    actor Utilisateur
    participant Main
    participant Fichier as FichierGrille
    participant Grille
    participant Jeu
    participant Console as ModeConsole
    participant Cellule
    participant Etat as EtatCellule
    participant Regle as RegleConway

    Utilisateur->>Main: fichier, mode console, limite
    Main->>Fichier: lire(chemin)
    Fichier->>Grille: créer(dimensions, états)
    Grille->>Cellule: créer les cellules
    Fichier-->>Main: grille initiale
    Main->>Jeu: créer(grille, règle)
    Main->>Console: executer(jeu, chemin, limite)
    Console->>Console: supprimer les anciens iteration_N.txt ciblés
    Console->>Fichier: ecrire(itération 0)
    loop Tant que la limite n'est pas atteinte et que la grille évolue
        Console->>Jeu: iterer()
        Jeu->>Grille: evoluer(règle)
        loop Pour chaque cellule
            Grille->>Cellule: estVivante()
            Cellule->>Etat: estVivante()
            Etat-->>Cellule: état courant
            Cellule-->>Grille: état courant
            Grille->>Regle: doitEtreVivante(état, voisines)
            Regle-->>Grille: état futur
        end
        Grille->>Cellule: changerEtat(état futur)
        Grille-->>Jeu: stabilité
        Jeu-->>Console: itération effectuée
        Console->>Fichier: ecrire(grille, fichier d'itération)
    end
    Console-->>Utilisateur: dossier de résultats et cause de l'arrêt
```
