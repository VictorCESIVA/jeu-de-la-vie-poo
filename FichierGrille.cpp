#include "FichierGrille.h"

#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <vector>

Grille FichierGrille::lire(const std::string& chemin) {
    std::ifstream fichier(chemin);
    if (!fichier) {
        throw std::runtime_error("Impossible d'ouvrir le fichier : " + chemin);
    }

    int lignes = 0;
    int colonnes = 0;
    if (!(fichier >> lignes >> colonnes) || lignes <= 0 || colonnes <= 0) {
        throw std::runtime_error("Dimensions absentes ou invalides dans : " + chemin);
    }

    const std::size_t nombreCellules = static_cast<std::size_t>(lignes) *
                                       static_cast<std::size_t>(colonnes);
    std::vector<int> etats;
    etats.reserve(nombreCellules);

    for (std::size_t indice = 0; indice < nombreCellules; ++indice) {
        int etat = 0;
        if (!(fichier >> etat) || (etat != 0 && etat != 1)) {
            throw std::runtime_error("La matrice doit contenir uniquement des 0 et des 1.");
        }
        etats.push_back(etat);
    }

    std::string contenuEnTrop;
    if (fichier >> contenuEnTrop) {
        throw std::runtime_error("La matrice contient plus de valeurs que prévu.");
    }

    return Grille(lignes, colonnes, etats);
}

void FichierGrille::ecrire(const Grille& grille, const std::string& chemin) {
    std::ofstream fichier(chemin);
    if (!fichier) {
        throw std::runtime_error("Impossible de créer le fichier : " + chemin);
    }

    fichier << grille.lignes() << ' ' << grille.colonnes() << '\n';
    for (int ligne = 0; ligne < grille.lignes(); ++ligne) {
        for (int colonne = 0; colonne < grille.colonnes(); ++colonne) {
            if (colonne > 0) {
                fichier << ' ';
            }
            fichier << (grille.estVivante(ligne, colonne) ? 1 : 0);
        }
        fichier << '\n';
    }

    fichier.flush();
    if (!fichier) {
        throw std::runtime_error("Erreur pendant l'écriture du fichier : " + chemin);
    }
}
