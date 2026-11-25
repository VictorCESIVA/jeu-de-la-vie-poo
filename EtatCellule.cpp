#include "EtatCellule.h"

bool EtatVivant::estVivante() const {
    return true;
}

std::unique_ptr<EtatCellule> EtatVivant::cloner() const {
    return std::make_unique<EtatVivant>();
}

bool EtatMort::estVivante() const {
    return false;
}

std::unique_ptr<EtatCellule> EtatMort::cloner() const {
    return std::make_unique<EtatMort>();
}
