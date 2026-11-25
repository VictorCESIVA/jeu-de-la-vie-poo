#pragma once

#include "Cellule.h"
#include "Regle.h"

#include <vector>

class Grille {
    int lignes_;
    int colonnes_;
    std::vector<Cellule> cellules_;

    int compterVoisinesVivantes(int ligne, int colonne) const;

public:
    Grille(int lignes, int colonnes, const std::vector<int>& etats);

    int lignes() const;
    int colonnes() const;
    bool estVivante(int ligne, int colonne) const;

    void modifierCellule(int ligne, int colonne, bool vivante);
    void basculerCellule(int ligne, int colonne);
    void effacer();

    bool evoluer(const Regle& regle);
    bool aMemesEtats(const Grille& autre) const;
};
