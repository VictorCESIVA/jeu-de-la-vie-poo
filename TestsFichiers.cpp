#include "TestsFichiers.h"

#include "FichierGrille.h"
#include "Jeu.h"
#include "ModeConsole.h"
#include "Regle.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace {
void verifier(bool condition, const std::string& message, bool& reussite) {
    if (!condition) {
        std::cerr << "Échec (fichiers) : " << message << '\n';
        reussite = false;
    }
}

void ecrireTexte(const std::filesystem::path& chemin, const std::string& contenu) {
    std::ofstream fichier(chemin, std::ios::binary);
    if (!fichier) {
        throw std::runtime_error("Impossible de créer le fichier de test : " + chemin.string());
    }
    fichier << contenu;
    if (!fichier) {
        throw std::runtime_error("Impossible d'écrire le fichier de test : " + chemin.string());
    }
}

std::vector<std::string> nomsDossier(const std::filesystem::path& dossier) {
    std::vector<std::string> noms;
    for (const auto& entree : std::filesystem::directory_iterator(dossier)) {
        noms.push_back(entree.path().filename().string());
    }
    std::sort(noms.begin(), noms.end());
    return noms;
}

void verifierContenuDossier(const std::filesystem::path& dossier,
                            std::vector<std::string> attendus,
                            const std::string& description, bool& reussite) {
    std::sort(attendus.begin(), attendus.end());
    const std::vector<std::string> obtenus = nomsDossier(dossier);
    if (obtenus == attendus) {
        return;
    }

    std::string message = description + " : attendu [";
    bool premier = true;
    for (const std::string& nom : attendus) {
        if (!premier) {
            message += ", ";
        }
        message += nom;
        premier = false;
    }
    message += "], obtenu [";
    premier = true;
    for (const std::string& nom : obtenus) {
        if (!premier) {
            message += ", ";
        }
        message += nom;
        premier = false;
    }
    message += ']';
    verifier(false, message, reussite);
}

void lancerConsole(const Grille& grille, const std::string& chemin, int iterations) {
    RegleConway regle;
    Jeu jeu(grille, regle);
    ModeConsole::executer(jeu, chemin, iterations);
}

template <typename Action, typename Predicat>
void verifierErreur(Action action, Predicat predicat, const std::string& description,
                    bool& reussite) {
    try {
        action();
    } catch (const std::exception& erreur) {
        const std::string message = erreur.what();
        if (!predicat(message)) {
            verifier(false, description + " : message inattendu : " + message, reussite);
        }
        return;
    } catch (...) {
        verifier(false, description + " : exception non standard", reussite);
        return;
    }
    verifier(false, description + " : aucune exception levée", reussite);
}

void verifierAllerRetour(const Grille& grille, const std::filesystem::path& chemin,
                         const std::string& description, bool& reussite) {
    FichierGrille::ecrire(grille, chemin.string());
    const Grille relue = FichierGrille::lire(chemin.string());
    verifier(relue.aMemesEtats(grille), description, reussite);
}

class DossierTemporaire {
    std::filesystem::path ancien_;
    std::filesystem::path racine_;

public:
    DossierTemporaire() : ancien_(std::filesystem::current_path()) {
        const std::filesystem::path base = std::filesystem::temp_directory_path();
        const auto horodatage = std::chrono::steady_clock::now().time_since_epoch().count();

        for (int tentative = 0; tentative < 100; ++tentative) {
            const std::filesystem::path candidat =
                base / ("jeu_de_la_vie_tests_" + std::to_string(horodatage + tentative));
            std::error_code erreur;
            if (std::filesystem::create_directory(candidat, erreur)) {
                racine_ = candidat;
                break;
            }
            if (erreur) {
                throw std::runtime_error("Impossible de créer le dossier de test : " +
                                         erreur.message());
            }
        }

        if (racine_.empty()) {
            throw std::runtime_error("Impossible de créer un dossier de test unique.");
        }

        try {
            std::filesystem::current_path(racine_);
        } catch (...) {
            std::error_code erreur;
            std::filesystem::remove_all(racine_, erreur);
            throw;
        }
    }

    ~DossierTemporaire() {
        std::error_code erreur;
        std::filesystem::current_path(ancien_, erreur);
        if (!erreur) {
            std::error_code erreurSuppression;
            std::filesystem::remove_all(racine_, erreurSuppression);
        }
    }

    DossierTemporaire(const DossierTemporaire&) = delete;
    DossierTemporaire& operator=(const DossierTemporaire&) = delete;
};
}

bool TestsFichiers::executer() {
    bool reussite = true;

    try {
        DossierTemporaire dossier;

        const Grille clignotant(3, 3, {0, 1, 0,
                                       0, 1, 0,
                                       0, 1, 0});

        lancerConsole(clignotant, "grille.txt", 6);
        verifierContenuDossier("grille.txt_out",
                               {"iteration_0.txt", "iteration_1.txt", "iteration_2.txt",
                                "iteration_3.txt", "iteration_4.txt", "iteration_5.txt",
                                "iteration_6.txt"},
                               "console longue : seuls les fichiers attendus existent",
                               reussite);

        ecrireTexte("grille.txt_out/notes.txt", "à conserver");
        ecrireTexte("grille.txt_out/iteration_notes.txt", "à conserver");
        ecrireTexte("grille.txt_out/autre.txt", "à conserver");
        verifier(std::filesystem::create_directory("grille.txt_out/iteration_99.txt"),
                 "création du dossier sentinelle iteration_99.txt", reussite);

        lancerConsole(clignotant, "grille.txt", 2);
        verifierContenuDossier("grille.txt_out",
                               {"iteration_0.txt", "iteration_1.txt", "iteration_2.txt",
                                "notes.txt", "iteration_notes.txt", "autre.txt",
                                "iteration_99.txt"},
                               "console longue puis courte : nettoyage ciblé", reussite);
        verifier(std::filesystem::is_directory("grille.txt_out/iteration_99.txt"),
                 "le dossier iteration_99.txt est préservé", reussite);

        const Grille iterationDeux =
            FichierGrille::lire("grille.txt_out/iteration_2.txt");
        verifier(iterationDeux.aMemesEtats(clignotant),
                 "l'itération 2 du clignotant est correcte", reussite);

        const Grille uneCellule(1, 1, {1});
        lancerConsole(uneCellule, "un.txt", 5);
        verifierContenuDossier("un.txt_out",
                               {"iteration_0.txt", "iteration_1.txt", "iteration_2.txt"},
                               "console 1x1", reussite);
        const Grille uneFinale = FichierGrille::lire("un.txt_out/iteration_2.txt");
        verifier(uneFinale.lignes() == 1 && uneFinale.colonnes() == 1 &&
                     !uneFinale.estVivante(0, 0),
                 "1x1 : la cellule meurt et la grille devient stable", reussite);

        const Grille ligne(1, 5, {1, 1, 1, 1, 1});
        lancerConsole(ligne, "ligne5.txt", 10);
        verifierContenuDossier("ligne5.txt_out",
                               {"iteration_0.txt", "iteration_1.txt", "iteration_2.txt",
                                "iteration_3.txt", "iteration_4.txt"},
                               "console 1x5", reussite);
        const Grille etapeUn = FichierGrille::lire("ligne5.txt_out/iteration_1.txt");
        verifier(etapeUn.aMemesEtats(Grille(1, 5, {0, 1, 1, 1, 0})),
                 "1x5 : évolution aux bords sans tore", reussite);
        const Grille ligneFinale = FichierGrille::lire("ligne5.txt_out/iteration_4.txt");
        bool ligneVide = true;
        for (int colonne = 0; colonne < ligneFinale.colonnes(); ++colonne) {
            if (ligneFinale.estVivante(0, colonne)) {
                ligneVide = false;
            }
        }
        verifier(ligneVide, "1x5 : la grille finit vide", reussite);

        verifierAllerRetour(Grille(1, 1, {0}), "aller_retour_1x1.txt",
                            "aller-retour lecture/écriture 1x1", reussite);
        verifierAllerRetour(Grille(1, 5, {1, 0, 0, 1, 1}), "aller_retour_1x5.txt",
                            "aller-retour lecture/écriture 1x5", reussite);
        verifierAllerRetour(Grille(5, 1, {1, 0, 0, 1, 1}), "aller_retour_5x1.txt",
                            "aller-retour lecture/écriture 5x1", reussite);

        ecrireTexte("matrice_incomplete.txt", "3 3\n1 0 1\n0 1\n0 0 0\n");
        verifierErreur(
            [&] { (void)FichierGrille::lire("matrice_incomplete.txt"); },
            [](const std::string& message) {
                return message.find("incomplète") != std::string::npos &&
                       message.find("illisible") != std::string::npos;
            },
            "diagnostic matrice incomplète", reussite);

        ecrireTexte("matrice_illisible.txt", "3 3\n1 0 x\n0 1 0\n0 0 0\n");
        verifierErreur(
            [&] { (void)FichierGrille::lire("matrice_illisible.txt"); },
            [](const std::string& message) {
                return message.find("illisible") != std::string::npos;
            },
            "diagnostic entier illisible", reussite);

        ecrireTexte("matrice_valeur_invalide.txt", "3 3\n1 0 2\n0 1 0\n0 0 0\n");
        verifierErreur(
            [&] { (void)FichierGrille::lire("matrice_valeur_invalide.txt"); },
            [](const std::string& message) {
                return message == "La matrice doit contenir uniquement des 0 et des 1.";
            },
            "diagnostic valeur hors 0/1", reussite);

        ecrireTexte("grandes_dimensions.txt", "100000 100000\n");
        verifierErreur(
            [&] { (void)FichierGrille::lire("grandes_dimensions.txt"); },
            [](const std::string& message) {
                return message.find("trop petit") != std::string::npos &&
                       message.find("bad_alloc") == std::string::npos &&
                       message.find("reserve") == std::string::npos;
            },
            "grandes dimensions dans un petit fichier", reussite);

        ecrireTexte("dimensions_max.txt", "2147483647 2147483647\n");
        verifierErreur(
            [&] { (void)FichierGrille::lire("dimensions_max.txt"); },
            [](const std::string& message) {
                return message.find("trop petit") != std::string::npos &&
                       message.find("bad_alloc") == std::string::npos &&
                       message.find("reserve") == std::string::npos;
            },
            "dimensions int max dans un petit fichier", reussite);
    } catch (const std::exception& erreur) {
        std::cerr << "Échec (fichiers) : exception inattendue : " << erreur.what() << '\n';
        return false;
    } catch (...) {
        std::cerr << "Échec (fichiers) : exception inattendue non standard.\n";
        return false;
    }

    return reussite;
}
