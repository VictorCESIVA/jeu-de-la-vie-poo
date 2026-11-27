#include "FichierGrille.h"
#include "Jeu.h"
#include "ModeConsole.h"
#include "ModeGraphique.h"
#include "Regle.h"
#include "Tests.h"

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void afficherUsage(std::ostream& sortie, const char* programme) {
    sortie << "Usage :\n"
           << "  " << programme << " <fichier> console [iterations=100]\n"
           << "  " << programme
           << " <fichier> graphique [delai_ms=100] [iterations_max=1000]\n"
           << "  " << programme << " --test\n"
           << "  " << programme << " --help | -h\n";
}

int lireNombrePositif(const char* texte, const std::string& nom) {
    const std::string chaine(texte);
    std::size_t position = 0;

    try {
        const int valeur = std::stoi(chaine, &position);
        if (position != chaine.size() || valeur <= 0) {
            throw std::invalid_argument("Nombre invalide");
        }
        return valeur;
    } catch (const std::exception&) {
        throw std::invalid_argument(nom + " doit être un entier strictement positif.");
    }
}
}

int main(int argc, char* argv[]) {
    if (argc == 2 && (std::string(argv[1]) == "--help" ||
                      std::string(argv[1]) == "-h")) {
        afficherUsage(std::cout, argv[0]);
        return 0;
    }
    if (argc == 2 && std::string(argv[1]) == "--test") {
        return Tests::executer();
    }
    if (argc < 3) {
        afficherUsage(std::cerr, argv[0]);
        return 1;
    }

    try {
        const std::string chemin = argv[1];
        const std::string mode = argv[2];
        if (mode != "console" && mode != "graphique") {
            afficherUsage(std::cerr, argv[0]);
            return 1;
        }
        if ((mode == "console" && argc > 4) || (mode == "graphique" && argc > 5)) {
            afficherUsage(std::cerr, argv[0]);
            return 1;
        }

        RegleConway regle;
        Jeu jeu(FichierGrille::lire(chemin), regle);

        if (mode == "console") {
            const int iterations = argc == 4
                                       ? lireNombrePositif(argv[3], "Le nombre d'itérations")
                                       : 100;
            ModeConsole::executer(jeu, chemin, iterations);
        } else {
            const int delai = argc >= 4
                                  ? lireNombrePositif(argv[3], "Le délai")
                                  : 100;
            const int iterationsMaximum = argc == 5
                                              ? lireNombrePositif(argv[4],
                                                                  "Le nombre maximal d'itérations")
                                              : 1000;
            ModeGraphique modeGraphique(delai);
            modeGraphique.executer(jeu, iterationsMaximum);
        }
    } catch (const std::exception& erreur) {
        std::cerr << "Erreur : " << erreur.what() << '\n';
        return 1;
    }

    return 0;
}
