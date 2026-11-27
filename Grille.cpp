#include "Grille.h"

#include <cstddef>
#include <stdexcept>
#include <thread>
#include <vector>

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

std::size_t Grille::indice(int ligne, int colonne) const {
    // Cast avant multiplication : le produit peut dépasser INT_MAX.
    return static_cast<std::size_t>(ligne) * static_cast<std::size_t>(colonnes_) +
           static_cast<std::size_t>(colonne);
}

bool Grille::estVivante(int ligne, int colonne) const {
    if (ligne < 0 || ligne >= lignes_ || colonne < 0 || colonne >= colonnes_) {
        throw std::out_of_range("Cellule hors de la grille.");
    }
    return cellules_[indice(ligne, colonne)].estVivante();
}

void Grille::modifierCellule(int ligne, int colonne, bool vivante) {
    if (ligne < 0 || ligne >= lignes_ || colonne < 0 || colonne >= colonnes_) {
        throw std::out_of_range("Cellule hors de la grille.");
    }

    cellules_[indice(ligne, colonne)].changerEtat(vivante);
}

bool Grille::estObstacle(int ligne, int colonne) const {
    if (ligne < 0 || ligne >= lignes_ || colonne < 0 || colonne >= colonnes_) {
        throw std::out_of_range("Cellule hors de la grille.");
    }
    return cellules_[indice(ligne, colonne)].estObstacle();
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

    cellules_[indice(ligne, colonne)].definirObstacle(obstacle);
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

namespace {
// En dessous de ces seuils, créer des threads coûte plus cher que le calcul.
constexpr std::size_t CELLULES_MIN_PARALLELE = 16384;
constexpr int LIGNES_MIN_PARALLELE = 16;
constexpr unsigned int THREADS_MAX = 16;
}

void Grille::calculerEtatsSuivants(const Regle& regle, int ligneDebut, int ligneFin,
                                   std::vector<unsigned char>& prochainsEtats) const {
    for (int ligne = ligneDebut; ligne < ligneFin; ++ligne) {
        for (int colonne = 0; colonne < colonnes_; ++colonne) {
            const std::size_t position = indice(ligne, colonne);
            const Cellule& cellule = cellules_[position];
            const bool vivante = cellule.estVivante();

            // Un obstacle ne suit jamais la règle : il garde exactement son état.
            if (cellule.estObstacle()) {
                prochainsEtats[position] = vivante ? 1 : 0;
                continue;
            }

            const int voisines = compterVoisinesVivantes(ligne, colonne);
            prochainsEtats[position] =
                regle.doitEtreVivante(vivante, voisines) ? 1 : 0;
        }
    }
}

bool Grille::evoluer(const Regle& regle) {
    // unsigned char et non vector<bool> : les threads écrivent des indices distincts.
    std::vector<unsigned char> prochainsEtats(cellules_.size());

    const unsigned int processeurs = std::thread::hardware_concurrency();
    if (cellules_.size() < CELLULES_MIN_PARALLELE || lignes_ < LIGNES_MIN_PARALLELE ||
        processeurs < 2) {
        calculerEtatsSuivants(regle, 0, lignes_, prochainsEtats);
    } else {
        unsigned int nombreThreads = processeurs;
        if (nombreThreads > THREADS_MAX) {
            nombreThreads = THREADS_MAX;
        }
        if (nombreThreads > static_cast<unsigned int>(lignes_)) {
            nombreThreads = static_cast<unsigned int>(lignes_);
        }

        std::vector<std::thread> threads;
        threads.reserve(nombreThreads);
        for (unsigned int numero = 0; numero < nombreThreads; ++numero) {
            const int debut = static_cast<int>(
                static_cast<long long>(lignes_) * numero / nombreThreads);
            const int fin = static_cast<int>(
                static_cast<long long>(lignes_) * (numero + 1) / nombreThreads);
            if (debut < fin) {
                threads.emplace_back([this, &regle, &prochainsEtats, debut, fin] {
                    calculerEtatsSuivants(regle, debut, fin, prochainsEtats);
                });
            }
        }
        for (std::thread& thread : threads) {
            thread.join();
        }
    }

    bool stable = true;
    for (int ligne = 0; ligne < lignes_; ++ligne) {
        for (int colonne = 0; colonne < colonnes_; ++colonne) {
            const std::size_t position = indice(ligne, colonne);
            const bool prochainEtat = prochainsEtats[position] != 0;
            if (cellules_[position].estVivante() != prochainEtat) {
                stable = false;
            }
            cellules_[position].changerEtat(prochainEtat);
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
