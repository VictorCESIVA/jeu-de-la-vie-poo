#include "Grille.h"

#include <cstddef>
#include <stdexcept>

Grille::Grille(int lignes, int colonnes, const std::vector<int>& etats)
    : lignes_(lignes), colonnes_(colonnes) {
    if (lignes <= 0 || colonnes <= 0) {
        throw std::invalid_argument("Les dimensions de la grille doivent être positives.");
    }

    const std::size_t nombreCellules = static_cast<std::size_t>(lignes_) *
                                       static_cast<std::size_t>(colonnes_);
    if (etats.size() != nombreCellules) {
        throw std::invalid_argument("Le nombre d'états ne correspond pas aux dimensions.");
    }

    cellules_.reserve(nombreCellules);
    for (int etat : etats) {
        if (etat != 0 && etat != 1) {
            throw std::invalid_argument("Une cellule doit valoir 0 ou 1.");
        }
        cellules_.push_back(Cellule(etat == 1));
    }
}

int Grille::lignes() const {
    return lignes_;
}

int Grille::colonnes() const {
    return colonnes_;
}

bool Grille::estVivante(int ligne, int colonne) const {
    if (ligne < 0 || ligne >= lignes_ || colonne < 0 || colonne >= colonnes_) {
        throw std::out_of_range("Cellule hors de la grille.");
    }
    return cellules_[static_cast<std::size_t>(ligne * colonnes_ + colonne)].estVivante();
}

void Grille::modifierCellule(int ligne, int colonne, bool vivante) {
    if (ligne < 0 || ligne >= lignes_ || colonne < 0 || colonne >= colonnes_) {
        throw std::out_of_range("Cellule hors de la grille.");
    }

    const std::size_t indice = static_cast<std::size_t>(ligne * colonnes_ + colonne);
    cellules_[indice].changerEtat(vivante);
}

bool Grille::estObstacle(int ligne, int colonne) const {
    if (ligne < 0 || ligne >= lignes_ || colonne < 0 || colonne >= colonnes_) {
        throw std::out_of_range("Cellule hors de la grille.");
    }
    return cellules_[static_cast<std::size_t>(ligne * colonnes_ + colonne)].estObstacle();
}

bool Grille::estTorique() const {
    return torique_;
}

void Grille::definirTorique(bool torique) {
    torique_ = torique;
}

void Grille::basculerCellule(int ligne, int colonne) {
    // estVivante lève out_of_range avant toute modification si les coordonnées sont invalides.
    modifierCellule(ligne, colonne, !estVivante(ligne, colonne));
}

void Grille::modifierObstacle(int ligne, int colonne, bool obstacle) {
    if (ligne < 0 || ligne >= lignes_ || colonne < 0 || colonne >= colonnes_) {
        throw std::out_of_range("Cellule hors de la grille.");
    }

    cellules_[static_cast<std::size_t>(ligne * colonnes_ + colonne)].definirObstacle(obstacle);
}

void Grille::basculerObstacle(int ligne, int colonne) {
    // estObstacle lève out_of_range avant toute modification si les coordonnées sont invalides.
    modifierObstacle(ligne, colonne, !estObstacle(ligne, colonne));
}

void Grille::effacer() {
    for (Cellule& cellule : cellules_) {
        cellule.changerEtat(false);
        cellule.definirObstacle(false);
    }
}

int Grille::compterVoisinesVivantes(int ligne, int colonne) const {
    int nombre = 0;

    for (int decalageLigne = -1; decalageLigne <= 1; ++decalageLigne) {
        for (int decalageColonne = -1; decalageColonne <= 1; ++decalageColonne) {
            if (decalageLigne == 0 && decalageColonne == 0) {
                continue;
            }

            int ligneVoisine = ligne + decalageLigne;
            int colonneVoisine = colonne + decalageColonne;
            if (torique_) {
                // Les décalages valent -1..1 : ajouter la dimension suffit pour rester positif.
                ligneVoisine = (ligneVoisine + lignes_) % lignes_;
                colonneVoisine = (colonneVoisine + colonnes_) % colonnes_;
            } else if (ligneVoisine < 0 || ligneVoisine >= lignes_ ||
                       colonneVoisine < 0 || colonneVoisine >= colonnes_) {
                continue;
            }

            if (estVivante(ligneVoisine, colonneVoisine)) {
                ++nombre;
            }
        }
    }

    return nombre;
}

bool Grille::evoluer(const Regle& regle) {
    std::vector<bool> prochainsEtats;
    prochainsEtats.reserve(cellules_.size());

    for (int ligne = 0; ligne < lignes_; ++ligne) {
        for (int colonne = 0; colonne < colonnes_; ++colonne) {
            const bool vivante = estVivante(ligne, colonne);
            // Un obstacle ne suit jamais la règle : il garde exactement son état.
            if (estObstacle(ligne, colonne)) {
                prochainsEtats.push_back(vivante);
                continue;
            }
            const int voisines = compterVoisinesVivantes(ligne, colonne);
            prochainsEtats.push_back(regle.doitEtreVivante(vivante, voisines));
        }
    }

    bool stable = true;
    for (int ligne = 0; ligne < lignes_; ++ligne) {
        for (int colonne = 0; colonne < colonnes_; ++colonne) {
            const std::size_t indice = static_cast<std::size_t>(ligne * colonnes_ + colonne);
            const bool prochainEtat = prochainsEtats[indice];
            if (estVivante(ligne, colonne) != prochainEtat) {
                stable = false;
            }
            cellules_[indice].changerEtat(prochainEtat);
        }
    }

    return stable;
}

bool Grille::aMemesEtats(const Grille& autre) const {
    if (lignes_ != autre.lignes_ || colonnes_ != autre.colonnes_) {
        return false;
    }

    for (int ligne = 0; ligne < lignes_; ++ligne) {
        for (int colonne = 0; colonne < colonnes_; ++colonne) {
            if (estVivante(ligne, colonne) != autre.estVivante(ligne, colonne)) {
                return false;
            }
        }
    }
    return true;
}
