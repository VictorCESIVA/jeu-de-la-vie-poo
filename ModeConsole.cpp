#include "ModeConsole.h"

#include "FichierGrille.h"

#include <cstddef>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {
bool estFichierIteration(const std::filesystem::path& chemin) {
    const std::string nom = chemin.filename().string();
    constexpr std::string_view prefixe = "iteration_";
    constexpr std::string_view suffixe = ".txt";

    if (nom.size() <= prefixe.size() + suffixe.size()) {
        return false;
    }
    if (nom.compare(0, prefixe.size(), prefixe.data(), prefixe.size()) != 0 ||
        nom.compare(nom.size() - suffixe.size(), suffixe.size(), suffixe.data(),
                    suffixe.size()) != 0) {
        return false;
    }

    const std::size_t debutChiffres = prefixe.size();
    const std::size_t finChiffres = nom.size() - suffixe.size();
    return nom.find_first_not_of("0123456789", debutChiffres) == finChiffres;
}

void supprimerIterationsExistantes(const std::filesystem::path& dossier) {
    std::vector<std::filesystem::path> aSupprimer;

    for (const auto& entree : std::filesystem::directory_iterator(dossier)) {
        if (std::filesystem::is_regular_file(entree.symlink_status()) &&
            estFichierIteration(entree.path())) {
            aSupprimer.push_back(entree.path());
        }
    }

    for (const auto& chemin : aSupprimer) {
        std::filesystem::remove(chemin);
    }
}
}

void ModeConsole::executer(Jeu& jeu, const std::string& cheminEntree,
                           int nombreIterations) {
    const std::filesystem::path entree(cheminEntree);
    const std::filesystem::path dossier(entree.filename().string() + "_out");
    std::filesystem::create_directories(dossier);
    supprimerIterationsExistantes(dossier);

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
