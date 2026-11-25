#include "Motif.h"

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

Motif::Motif(std::string nom, const std::vector<std::vector<int>>& cellules)
    : nom_(std::move(nom)), cellules_(cellules) {
    if (nom_.empty()) {
        throw std::invalid_argument("Le nom du motif ne peut pas être vide.");
    }
    if (cellules_.empty()) {
        throw std::invalid_argument("La matrice du motif ne peut pas être vide.");
    }
    if (cellules_.front().empty()) {
        throw std::invalid_argument("Le motif doit contenir au moins une colonne.");
    }

    const auto limite = static_cast<std::size_t>(std::numeric_limits<int>::max());
    if (cellules_.size() > limite || cellules_.front().size() > limite) {
        throw std::invalid_argument("Les dimensions du motif dépassent la capacité de int.");
    }

    const auto nombreColonnes = cellules_.front().size();
    for (const auto& ligne : cellules_) {
        if (ligne.size() != nombreColonnes) {
            throw std::invalid_argument("La matrice du motif doit être rectangulaire.");
        }
        for (int cellule : ligne) {
            if (cellule != 0 && cellule != 1) {
                throw std::invalid_argument("Les cellules du motif doivent valoir 0 ou 1.");
            }
        }
    }
}

const std::string& Motif::nom() const {
    return nom_;
}

int Motif::lignes() const {
    return static_cast<int>(cellules_.size());
}

int Motif::colonnes() const {
    return static_cast<int>(cellules_.front().size());
}

bool Motif::estVivante(int ligne, int colonne) const {
    return cellules_.at(ligne).at(colonne) == 1;
}

std::vector<Motif> Motif::predefinis() {
    return {
        Motif("Planeur", {
            {0, 1, 0},
            {0, 0, 1},
            {1, 1, 1}
        }),
        Motif("Clignotant", {
            {0, 1, 0},
            {0, 1, 0},
            {0, 1, 0}
        }),
        Motif("Bloc", {
            {1, 1},
            {1, 1}
        }),
        Motif("Crapaud", {
            {0, 0, 0, 0},
            {0, 1, 1, 1},
            {1, 1, 1, 0},
            {0, 0, 0, 0}
        })
    };
}
