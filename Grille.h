#pragma once

#include "Cellule.h"
#include "Regle.h"

#include <vector>

class Grille {
    int lignes_;
    int colonnes_;
    bool torique_ = false;
    std::vector<Cellule> cellules_;

    int compterVoisinesVivantes(int ligne, int colonne) const;

public:
    Grille(int lignes, int colonnes, const std::vector<int>& etats);

    int lignes() const;
    int colonnes() const;
    bool estVivante(int ligne, int colonne) const;
    bool estObstacle(int ligne, int colonne) const;
    bool estTorique() const;
    void definirTorique(bool torique);

    void modifierCellule(int ligne, int colonne, bool vivante);
    void basculerCellule(int ligne, int colonne);
    void modifierObstacle(int ligne, int colonne, bool obstacle);
    void basculerObstacle(int ligne, int colonne);
    void effacer();

    bool evoluer(const Regle& regle);
    bool aMemesEtats(const Grille& autre) const;
};
