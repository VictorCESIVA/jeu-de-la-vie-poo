#pragma once

#include "Jeu.h"

#include <string>

class ModeConsole {
public:
    static void executer(Jeu& jeu, const std::string& cheminEntree,
                         int nombreIterations);
};
