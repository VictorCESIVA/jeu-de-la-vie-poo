#include "Cellule.h"

Cellule::Cellule(bool vivante) {
    changerEtat(vivante);
}

Cellule::Cellule(const Cellule& autre) : etat_(autre.etat_->cloner()) {
}

Cellule& Cellule::operator=(const Cellule& autre) {
    if (this != &autre) {
        etat_ = autre.etat_->cloner();
    }
    return *this;
}

bool Cellule::estVivante() const {
    return etat_->estVivante();
}

void Cellule::changerEtat(bool vivante) {
    if (etat_ && etat_->estVivante() == vivante) {
        return;
    }

    if (vivante) {
        etat_ = std::make_unique<EtatVivant>();
    } else {
        etat_ = std::make_unique<EtatMort>();
    }
}
