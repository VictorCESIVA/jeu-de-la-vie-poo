#pragma once

#include "Grille.h"
#include "Motif.h"
#include "Regle.h"

class Jeu {
    Grille grille_;
    Grille initiale_;
    const Regle& regle_;
    int iteration_ = 0;
    bool stable_ = false;

public:
    Jeu(Grille grille, const Regle& regle);

    void iterer();
    int iteration() const;
    bool estStable() const;
    const Grille& grille() const;

    void basculerCellule(int ligne, int colonne);
    void modifierCellule(int ligne, int colonne, bool vivante);
    void effacer();
    void reinitialiser();
    bool placerMotif(const Motif& motif, int ligneCentre, int colonneCentre);
};
