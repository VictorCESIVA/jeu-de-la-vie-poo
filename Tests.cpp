#include "Tests.h"

#include "Jeu.h"
#include "Motif.h"
#include "Regle.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
void verifier(bool condition, const std::string& message, bool& reussite) {
    if (!condition) {
        std::cerr << "Échec : " << message << '\n';
        reussite = false;
    }
}

int compterVivantes(const Motif& motif) {
    int nombre = 0;
    for (int ligne = 0; ligne < motif.lignes(); ++ligne) {
        for (int colonne = 0; colonne < motif.colonnes(); ++colonne) {
            if (motif.estVivante(ligne, colonne)) {
                ++nombre;
            }
        }
    }
    return nombre;
}

Grille grilleVide(int lignes, int colonnes) {
    return Grille(lignes, colonnes, std::vector<int>(static_cast<std::size_t>(lignes) *
                                                         static_cast<std::size_t>(colonnes),
                                                     0));
}
}

bool Tests::verifierGrille(const Grille& initiale, const Grille& attendue,
                           int nombreIterations) {
    RegleConway regle;
    Jeu jeu(initiale, regle);

    for (int numero = 0; numero < nombreIterations && !jeu.estStable(); ++numero) {
        jeu.iterer();
    }
    return jeu.grille().aMemesEtats(attendue);
}

int Tests::executer() {
    const Grille clignotantInitial(3, 3, {0, 1, 0,
                                         0, 1, 0,
                                         0, 1, 0});
    const Grille clignotantHorizontal(3, 3, {0, 0, 0,
                                            1, 1, 1,
                                            0, 0, 0});
    const Grille blocStable(4, 4, {0, 0, 0, 0,
                                  0, 1, 1, 0,
                                  0, 1, 1, 0,
                                  0, 0, 0, 0});

    bool reussite = true;
    if (!verifierGrille(clignotantInitial, clignotantHorizontal, 1)) {
        std::cerr << "Échec : le clignotant après une itération.\n";
        reussite = false;
    }
    if (!verifierGrille(clignotantInitial, clignotantInitial, 2)) {
        std::cerr << "Échec : le clignotant après deux itérations.\n";
        reussite = false;
    }
    if (!verifierGrille(blocStable, blocStable, 5)) {
        std::cerr << "Échec : le bloc stable après cinq itérations.\n";
        reussite = false;
    }

    RegleConway regle;
    const std::vector<Motif> motifs = Motif::predefinis();

    verifier(motifs.size() == 4, "il existe quatre motifs prédéfinis", reussite);
    if (motifs.size() == 4) {
        verifier(motifs[0].nom() == "Planeur" && motifs[0].lignes() == 3 &&
                     motifs[0].colonnes() == 3 && compterVivantes(motifs[0]) == 5,
                 "le planeur est un motif 3x3 de cinq cellules", reussite);
        verifier(motifs[1].nom() == "Clignotant" && motifs[1].lignes() == 3 &&
                     motifs[1].colonnes() == 3 && compterVivantes(motifs[1]) == 3,
                 "le clignotant est un motif 3x3 de trois cellules", reussite);
        verifier(motifs[2].nom() == "Bloc" && motifs[2].lignes() == 2 &&
                     motifs[2].colonnes() == 2 && compterVivantes(motifs[2]) == 4,
                 "le bloc est un motif 2x2 de quatre cellules", reussite);
        verifier(motifs[3].nom() == "Crapaud" && motifs[3].lignes() == 4 &&
                     motifs[3].colonnes() == 4 && compterVivantes(motifs[3]) == 6,
                 "le crapaud est un motif 4x4 de six cellules", reussite);
    }

    {
        Jeu jeu(grilleVide(5, 5), regle);
        verifier(!jeu.estStable(), "une grille neuve n'est pas stable", reussite);
        jeu.iterer();
        verifier(jeu.estStable(), "une grille vide devient stable en une itération",
                 reussite);

        jeu.basculerCellule(2, 2);
        verifier(jeu.grille().estVivante(2, 2), "la bascule rend la cellule vivante",
                 reussite);
        verifier(!jeu.estStable(), "une modification lève la stabilité", reussite);
        jeu.basculerCellule(2, 2);
        verifier(!jeu.grille().estVivante(2, 2),
                 "une seconde bascule rend la cellule morte", reussite);

        jeu.basculerCellule(2, 2);
        jeu.iterer();
        verifier(!jeu.grille().estVivante(2, 2), "une cellule isolée meurt", reussite);
        verifier(jeu.iteration() == 2, "la reprise après modification itère à nouveau",
                 reussite);
        jeu.iterer();
        verifier(jeu.estStable(), "la grille redevient stable après la disparition",
                 reussite);
    }

    {
        Jeu jeu(grilleVide(3, 3), regle);
        jeu.basculerCellule(1, 1);

        bool refusee = false;
        try {
            jeu.basculerCellule(1, -1);
        } catch (const std::out_of_range&) {
            refusee = true;
        }
        verifier(refusee, "une coordonnée négative est refusée", reussite);
        verifier(!jeu.grille().estVivante(0, 2),
                 "une bascule invalide ne modifie pas la case (0,2)", reussite);
        verifier(jeu.grille().estVivante(1, 1),
                 "une bascule invalide ne modifie pas la case valide", reussite);
    }

    {
        Jeu jeu(clignotantInitial, regle);
        jeu.iterer();
        verifier(!jeu.estStable(), "le clignotant oscille", reussite);

        jeu.effacer();
        verifier(jeu.grille().aMemesEtats(grilleVide(3, 3)),
                 "l'effacement tue toutes les cellules", reussite);
        verifier(!jeu.estStable(), "l'effacement lève la stabilité", reussite);
        verifier(jeu.iteration() == 0, "l'effacement remet le compteur à zéro",
                 reussite);

        jeu.iterer();
        verifier(jeu.estStable(),
                 "une itération après effacement détecte la stabilité", reussite);
        verifier(jeu.iteration() == 1, "l'itération après effacement repart de zéro",
                 reussite);
    }

    {
        Jeu jeu(clignotantInitial, regle);
        jeu.iterer();
        jeu.iterer();
        jeu.basculerCellule(0, 0);

        jeu.reinitialiser();
        verifier(jeu.iteration() == 0, "la réinitialisation remet le compteur à zéro",
                 reussite);
        verifier(jeu.grille().aMemesEtats(clignotantInitial),
                 "la réinitialisation restaure la grille initiale", reussite);
        verifier(!jeu.estStable(), "la réinitialisation lève la stabilité", reussite);

        jeu.iterer();
        verifier(jeu.iteration() == 1, "une itération reste possible après reset",
                 reussite);
    }

    if (motifs.size() == 4) {
        Jeu jeu(grilleVide(5, 5), regle);
        verifier(!jeu.placerMotif(motifs[0], 0, 0),
                 "un planeur ne tient pas dans le coin supérieur gauche", reussite);
        verifier(!jeu.placerMotif(motifs[0], 4, 4),
                 "un planeur ne tient pas dans le coin inférieur droit", reussite);
        verifier(jeu.grille().aMemesEtats(grilleVide(5, 5)),
                 "un placement refusé ne modifie pas la grille", reussite);

        verifier(jeu.placerMotif(motifs[1], 2, 2),
                 "un clignotant tient au centre d'une grille 5x5", reussite);
        const Grille clignotantCentre(5, 5, {0, 0, 0, 0, 0,
                                             0, 0, 1, 0, 0,
                                             0, 0, 1, 0, 0,
                                             0, 0, 1, 0, 0,
                                             0, 0, 0, 0, 0});
        verifier(jeu.grille().aMemesEtats(clignotantCentre),
                 "le motif est centré autour de la case cliquée", reussite);

        jeu.iterer();
        const Grille clignotantHorizontalCentre(5, 5, {0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0,
                                                       0, 1, 1, 1, 0,
                                                       0, 0, 0, 0, 0,
                                                       0, 0, 0, 0, 0});
        verifier(jeu.grille().aMemesEtats(clignotantHorizontalCentre),
                 "un motif placé évolue selon les règles", reussite);
    }

    if (motifs.size() == 4) {
        Jeu jeu(grilleVide(10, 10), regle);
        verifier(jeu.placerMotif(motifs[0], 3, 3),
                 "un planeur tient au centre d'une grille 10x10", reussite);
        for (int numero = 0; numero < 4; ++numero) {
            jeu.iterer();
        }

        bool planeurTranslate = true;
        for (int ligne = 0; ligne < jeu.grille().lignes(); ++ligne) {
            for (int colonne = 0; colonne < jeu.grille().colonnes(); ++colonne) {
                const bool attendue = (ligne == 3 && colonne == 4) ||
                                      (ligne == 4 && colonne == 5) ||
                                      (ligne == 5 && colonne == 3) ||
                                      (ligne == 5 && colonne == 4) ||
                                      (ligne == 5 && colonne == 5);
                if (jeu.grille().estVivante(ligne, colonne) != attendue) {
                    planeurTranslate = false;
                }
            }
        }
        verifier(planeurTranslate,
                 "le planeur est translaté d'une case en diagonale après quatre itérations",
                 reussite);
        verifier(!jeu.estStable(), "le planeur reste instable", reussite);
    }

    if (motifs.size() == 4) {
        Jeu jeu(grilleVide(6, 6), regle);
        jeu.iterer();
        verifier(jeu.estStable(), "la grille vide est stable avant placement", reussite);

        verifier(jeu.placerMotif(motifs[2], 2, 2), "un bloc peut être placé", reussite);
        verifier(!jeu.estStable(), "un placement lève la stabilité", reussite);
        jeu.iterer();
        verifier(jeu.estStable(), "le bloc placé est stable après une itération",
                 reussite);
        verifier(jeu.grille().estVivante(1, 1) && jeu.grille().estVivante(1, 2) &&
                     jeu.grille().estVivante(2, 1) && jeu.grille().estVivante(2, 2),
                 "le bloc placé conserve ses quatre cellules", reussite);
    }

    if (reussite) {
        std::cout << "Tous les tests unitaires sont passés.\n";
        return 0;
    }
    return 1;
}
