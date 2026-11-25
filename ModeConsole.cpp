#include "ModeConsole.h"

#include "FichierGrille.h"

#include <filesystem>
#include <iostream>
#include <string>

void ModeConsole::executer(Jeu& jeu, const std::string& cheminEntree,
                           int nombreIterations) {
    const std::filesystem::path entree(cheminEntree);
    const std::filesystem::path dossier(entree.filename().string() + "_out");
    std::filesystem::create_directories(dossier);

    FichierGrille::ecrire(jeu.grille(), (dossier / "iteration_0.txt").string());

    while (jeu.iteration() < nombreIterations && !jeu.estStable()) {
        jeu.iterer();
        const std::string nomFichier = "iteration_" +
                                       std::to_string(jeu.iteration()) + ".txt";
        FichierGrille::ecrire(jeu.grille(), (dossier / nomFichier).string());
    }

    std::cout << "Résultats écrits dans " << dossier.string() << " après "
              << jeu.iteration() << " itération(s).\n";
    if (jeu.estStable()) {
        std::cout << "La grille ne change plus : arrêt automatique.\n";
    }
}
