#pragma once

#include "Grille.h"

class Tests {
    static bool verifierGrille(const Grille& initiale, const Grille& attendue,
                               int nombreIterations);

public:
    static int executer();
};
