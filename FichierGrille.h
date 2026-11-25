#pragma once

#include "Grille.h"

#include <string>

class FichierGrille {
public:
    static Grille lire(const std::string& chemin);
    static void ecrire(const Grille& grille, const std::string& chemin);
};
