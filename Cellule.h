#pragma once

#include "EtatCellule.h"

#include <memory>

class Cellule {
    std::unique_ptr<EtatCellule> etat_;
    bool obstacle_ = false;

public:
    explicit Cellule(bool vivante = false);
    Cellule(const Cellule& autre);
    Cellule& operator=(const Cellule& autre);

    bool estVivante() const;
    void changerEtat(bool vivante);
    bool estObstacle() const;
    void definirObstacle(bool obstacle);
};
