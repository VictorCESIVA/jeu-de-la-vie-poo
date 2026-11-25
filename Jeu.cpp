#include "Jeu.h"

#include <utility>

Jeu::Jeu(Grille grille, const Regle& regle)
    : grille_(std::move(grille)), initiale_(grille_), regle_(regle) {
}

void Jeu::iterer() {
    if (stable_) {
        return;
    }

    stable_ = grille_.evoluer(regle_);
    ++iteration_;
}

int Jeu::iteration() const {
    return iteration_;
}

bool Jeu::estStable() const {
    return stable_;
}

const Grille& Jeu::grille() const {
    return grille_;
}

void Jeu::basculerCellule(int ligne, int colonne) {
    modifierCellule(ligne, colonne, !grille_.estVivante(ligne, colonne));
}

void Jeu::modifierCellule(int ligne, int colonne, bool vivante) {
    grille_.modifierCellule(ligne, colonne, vivante);
    stable_ = false;
}

void Jeu::effacer() {
    grille_.effacer();
    iteration_ = 0;
    stable_ = false;
}

void Jeu::reinitialiser() {
    grille_ = initiale_;
    iteration_ = 0;
    stable_ = false;
}

bool Jeu::placerMotif(const Motif& motif, int ligneCentre, int colonneCentre) {
    const int ligneDepart = ligneCentre - motif.lignes() / 2;
    const int colonneDepart = colonneCentre - motif.colonnes() / 2;

    if (ligneDepart < 0 || colonneDepart < 0 ||
        ligneDepart + motif.lignes() > grille_.lignes() ||
        colonneDepart + motif.colonnes() > grille_.colonnes()) {
        return false;
    }

    // Seules les cellules vivantes du motif sont posées, le reste de la grille est conservé.
    for (int ligne = 0; ligne < motif.lignes(); ++ligne) {
        for (int colonne = 0; colonne < motif.colonnes(); ++colonne) {
            if (motif.estVivante(ligne, colonne)) {
                grille_.modifierCellule(ligneDepart + ligne, colonneDepart + colonne, true);
            }
        }
    }

    stable_ = false;
    return true;
}
