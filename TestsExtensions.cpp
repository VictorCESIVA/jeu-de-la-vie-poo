#include "TestsExtensions.h"

#include "Grille.h"
#include "Jeu.h"
#include "Regle.h"

#include <atomic>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

void verifier(bool condition, const std::string& message, bool& reussite) {
    if (!condition) {
        std::cerr << "Échec : " << message << '\n';
        reussite = false;
    }
}

Grille grilleVide(int lignes, int colonnes) {
    return Grille(lignes, colonnes,
                  std::vector<int>(static_cast<std::size_t>(lignes) *
                                       static_cast<std::size_t>(colonnes),
                                   0));
}

Grille grilleAvec(int lignes, int colonnes,
                  std::initializer_list<std::pair<int, int>> vivantes) {
    Grille grille = grilleVide(lignes, colonnes);
    for (const auto& [ligne, colonne] : vivantes) {
        grille.modifierCellule(ligne, colonne, true);
    }
    return grille;
}

bool memesCellulesEtObstacles(const Grille& gauche, const Grille& droite) {
    if (gauche.lignes() != droite.lignes() || gauche.colonnes() != droite.colonnes()) {
        return false;
    }

    for (int ligne = 0; ligne < gauche.lignes(); ++ligne) {
        for (int colonne = 0; colonne < gauche.colonnes(); ++colonne) {
            if (gauche.estVivante(ligne, colonne) != droite.estVivante(ligne, colonne) ||
                gauche.estObstacle(ligne, colonne) !=
                    droite.estObstacle(ligne, colonne)) {
                return false;
            }
        }
    }
    return true;
}

bool grilleEntierementVide(const Grille& grille) {
    for (int ligne = 0; ligne < grille.lignes(); ++ligne) {
        for (int colonne = 0; colonne < grille.colonnes(); ++colonne) {
            if (grille.estVivante(ligne, colonne) ||
                grille.estObstacle(ligne, colonne)) {
                return false;
            }
        }
    }
    return true;
}

template <typename Action>
void verifierHorsBornes(Action action, const std::string& description, bool& reussite) {
    try {
        action();
    } catch (const std::out_of_range& erreur) {
        verifier(std::string(erreur.what()) == "Cellule hors de la grille.",
                 description + " : message inattendu : " + erreur.what(), reussite);
        return;
    } catch (const std::exception& erreur) {
        verifier(false, description + " : type d'exception inattendu : " + erreur.what(),
                 reussite);
        return;
    } catch (...) {
        verifier(false, description + " : exception non standard", reussite);
        return;
    }
    verifier(false, description + " : aucune exception levée", reussite);
}

class RegleEspionne final : public Regle {
    bool resultat_;
    mutable std::atomic<int> appels_{0};

public:
    explicit RegleEspionne(bool resultat) : resultat_(resultat) {
    }

    bool doitEtreVivante(bool, int) const override {
        ++appels_;
        return resultat_;
    }

    int appels() const {
        return appels_.load();
    }
};

void testerToreEtRepli(bool& reussite) {
    const RegleConway regle;

    verifier(!grilleVide(3, 3).estTorique(),
             "une grille doit être non torique par défaut", reussite);

    // Cible (1,2) morte : ses trois voisines vivantes sont (0,2), (2,2) et (1,0),
    // cette dernière seulement grâce au repli horizontal.
    Grille horizontal = grilleAvec(3, 3, {{1, 0}, {0, 2}, {2, 2}});
    horizontal.definirTorique(true);
    horizontal.evoluer(regle);
    verifier(horizontal.estVivante(1, 2),
             "en tore, la voisine (1,0) doit se replier en colonne et faire naître (1,2)",
             reussite);

    Grille horizontalSansTore = grilleAvec(3, 3, {{1, 0}, {0, 2}, {2, 2}});
    horizontalSansTore.evoluer(regle);
    verifier(!horizontalSansTore.estVivante(1, 2),
             "hors tore, (1,2) n'a que deux voisines et doit rester morte", reussite);

    // Cible (2,1) morte : voisines (2,0), (2,2) et (0,1), cette dernière par repli vertical.
    Grille vertical = grilleAvec(3, 3, {{0, 1}, {2, 0}, {2, 2}});
    vertical.definirTorique(true);
    vertical.evoluer(regle);
    verifier(vertical.estVivante(2, 1),
             "en tore, la voisine (0,1) doit se replier en ligne et faire naître (2,1)",
             reussite);

    Grille verticalSansTore = grilleAvec(3, 3, {{0, 1}, {2, 0}, {2, 2}});
    verticalSansTore.evoluer(regle);
    verifier(!verticalSansTore.estVivante(2, 1),
             "hors tore, (2,1) n'a que deux voisines et doit rester morte", reussite);

    // Cible (0,0) morte : voisines (0,1), (1,0) et (2,2), cette dernière par repli de coin.
    Grille coin = grilleAvec(3, 3, {{2, 2}, {0, 1}, {1, 0}});
    coin.definirTorique(true);
    coin.evoluer(regle);
    verifier(coin.estVivante(0, 0),
             "en tore, la voisine (2,2) doit se replier en coin et faire naître (0,0)",
             reussite);

    Grille coinSansTore = grilleAvec(3, 3, {{2, 2}, {0, 1}, {1, 0}});
    coinSansTore.evoluer(regle);
    verifier(!coinSansTore.estVivante(0, 0),
             "hors tore, (0,0) n'a que deux voisines et doit rester morte", reussite);

    // En tore 3x3, chacun des huit décalages atteint une cellule distincte autre que
    // la cellule elle-même : les trois vivantes de la diagonale ont deux voisines et
    // survivent, les six mortes en ont trois et naissent.
    Grille diagonale = grilleAvec(3, 3, {{0, 0}, {1, 1}, {2, 2}});
    diagonale.definirTorique(true);
    diagonale.evoluer(regle);
    const Grille attendueTore(3, 3, {1, 1, 1,
                                     1, 1, 1,
                                     1, 1, 1});
    verifier(memesCellulesEtObstacles(diagonale, attendueTore),
             "en tore 3x3, la diagonale vivante doit donner une grille entièrement vivante",
             reussite);

    // Même état initial, mais le tore est désactivé : seuls les trois vivantes ont
    // chacune deux voisines, aucune naissance n'a lieu.
    for (int ligne = 0; ligne < diagonale.lignes(); ++ligne) {
        for (int colonne = 0; colonne < diagonale.colonnes(); ++colonne) {
            diagonale.modifierCellule(ligne, colonne, false);
        }
    }
    diagonale.modifierCellule(0, 0, true);
    diagonale.modifierCellule(1, 1, true);
    diagonale.modifierCellule(2, 2, true);
    diagonale.definirTorique(false);
    diagonale.evoluer(regle);
    const Grille attendueHorsTore(3, 3, {0, 0, 0,
                                         0, 1, 0,
                                         0, 0, 0});
    verifier(!diagonale.estTorique(),
             "definirTorique(false) doit désactiver le tore", reussite);
    verifier(memesCellulesEtObstacles(diagonale, attendueHorsTore),
             "definirTorique(false) doit rétablir le voisinage borné non torique",
             reussite);
}

void testerTore2x2Multiplicites(bool& reussite) {
    const RegleConway regle;

    // En tore 2x2, les huit décalages retombent sur trois cellules distinctes : la
    // diagonale opposée est comptée 4 fois, chaque orthogonale 2 fois.
    Grille diagonale = grilleAvec(2, 2, {{0, 0}, {1, 1}});
    diagonale.definirTorique(true);
    diagonale.evoluer(regle);
    verifier(memesCellulesEtObstacles(diagonale, grilleVide(2, 2)),
             "en tore 2x2, deux vivantes en diagonale se comptent 4 fois et meurent",
             reussite);

    Grille voisines = grilleAvec(2, 2, {{0, 0}, {0, 1}});
    voisines.definirTorique(true);
    verifier(voisines.evoluer(regle),
             "en tore 2x2, deux vivantes adjacentes ont 2 voisines et sont stables",
             reussite);
    verifier(memesCellulesEtObstacles(voisines, grilleAvec(2, 2, {{0, 0}, {0, 1}})),
             "en tore 2x2, la paire adjacente doit rester inchangée", reussite);

    Grille seule = grilleAvec(2, 2, {{0, 0}});
    seule.definirTorique(true);
    seule.evoluer(regle);
    verifier(memesCellulesEtObstacles(seule, grilleVide(2, 2)),
             "en tore 2x2, une vivante isolée doit mourir", reussite);

    Grille pleine = grilleAvec(2, 2, {{0, 0}, {0, 1}, {1, 0}, {1, 1}});
    pleine.definirTorique(true);
    pleine.evoluer(regle);
    verifier(memesCellulesEtObstacles(pleine, grilleVide(2, 2)),
             "en tore 2x2, quatre vivantes comptent chacune 8 voisines et meurent",
             reussite);
}

void testerObstacles(bool& reussite) {
    const RegleConway regle;

    // Un obstacle vivant isolé ne doit subir aucune règle, même une règle qui le tuerait.
    Grille isole = grilleVide(3, 3);
    isole.modifierCellule(1, 1, true);
    isole.modifierObstacle(1, 1, true);
    const RegleEspionne regleQuiTue(false);
    isole.evoluer(regleQuiTue);
    verifier(isole.estVivante(1, 1),
             "un obstacle vivant sans voisine doit rester vivant", reussite);
    verifier(regleQuiTue.appels() == 8,
             "la règle ne doit pas être appelée pour l'obstacle vivant, seulement pour les 8 cellules normales",
             reussite);

    // Un obstacle mort entouré de trois vivantes ne doit pas naître non plus.
    Grille mort = grilleAvec(3, 3, {{0, 0}, {0, 1}, {1, 0}});
    mort.modifierObstacle(1, 1, true);
    const RegleEspionne regleQuiFaitNaitre(true);
    mort.evoluer(regleQuiFaitNaitre);
    verifier(!mort.estVivante(1, 1),
             "un obstacle mort avec exactement trois voisines doit rester mort", reussite);
    verifier(regleQuiFaitNaitre.appels() == 8,
             "la règle ne doit pas être appelée pour l'obstacle mort, seulement pour les 8 cellules normales",
             reussite);

    // Une naissance normale doit compter l'obstacle vivant : (1,1) a trois voisines.
    Grille naissance = grilleAvec(3, 3, {{0, 0}, {0, 1}});
    naissance.modifierCellule(0, 2, true);
    naissance.modifierObstacle(0, 2, true);
    naissance.evoluer(regle);
    verifier(naissance.estVivante(1, 1),
             "un obstacle vivant doit compter parmi les trois voisines qui font naître (1,1)",
             reussite);

    // Une survie normale doit compter l'obstacle vivant : (1,1) a deux voisines.
    Grille survie = grilleAvec(3, 3, {{1, 1}, {0, 0}});
    survie.modifierCellule(0, 2, true);
    survie.modifierObstacle(0, 2, true);
    survie.evoluer(regle);
    verifier(survie.estVivante(1, 1),
             "un obstacle vivant doit compter parmi les deux voisines qui maintiennent (1,1) en vie",
             reussite);
}

void testerObstacleDeBordTorique(bool& reussite) {
    const RegleConway regle;

    // Obstacle vivant en (0,0) : en tore 3x3 il est voisin de (2,2) par repli de coin.
    // (2,1) et (1,2) complètent les trois voisines : (2,2) naît, l'obstacle est conservé.
    Grille tore = grilleAvec(3, 3, {{2, 1}, {1, 2}});
    tore.modifierCellule(0, 0, true);
    tore.modifierObstacle(0, 0, true);
    tore.definirTorique(true);
    tore.evoluer(regle);
    verifier(tore.estObstacle(0, 0) && tore.estVivante(0, 0),
             "un obstacle vivant sur un coin du tore doit être conservé", reussite);
    verifier(tore.estVivante(2, 2),
             "un obstacle vivant de coin doit compter comme voisine repliée de (2,2)",
             reussite);

    // Sans tore, (0,0) n'est plus voisin de (2,2) : deux voisines seulement, pas de naissance.
    Grille bornee = grilleAvec(3, 3, {{2, 1}, {1, 2}});
    bornee.modifierCellule(0, 0, true);
    bornee.modifierObstacle(0, 0, true);
    bornee.evoluer(regle);
    verifier(!bornee.estVivante(2, 2),
             "hors tore, l'obstacle de coin ne doit pas compter pour (2,2)", reussite);
    verifier(bornee.estObstacle(0, 0) && bornee.estVivante(0, 0),
             "hors tore, l'obstacle vivant de coin doit aussi être conservé", reussite);
}

void testerAccesHorsBornes(bool& reussite) {
    Grille grille = grilleAvec(2, 3, {{0, 0}, {1, 2}});
    grille.modifierObstacle(0, 2, true);
    const Grille reference = grille;

    const std::pair<int, int> coordonneesInvalides[] = {
        {-1, 0}, {0, -1}, {2, 0}, {0, 3}, {2, 3}, {-1, -1}};

    for (const auto& [ligne, colonne] : coordonneesInvalides) {
        const std::string position =
            "(" + std::to_string(ligne) + "," + std::to_string(colonne) + ")";
        verifierHorsBornes([&] { (void)grille.estObstacle(ligne, colonne); },
                           "estObstacle hors bornes " + position, reussite);
        verifierHorsBornes([&] { grille.modifierObstacle(ligne, colonne, true); },
                           "modifierObstacle hors bornes " + position, reussite);
        verifierHorsBornes([&] { grille.basculerObstacle(ligne, colonne); },
                           "basculerObstacle hors bornes " + position, reussite);
    }

    verifier(memesCellulesEtObstacles(grille, reference),
             "les accès hors bornes ne doivent modifier ni états ni obstacles", reussite);
}

void testerBasculeEtStabilite(bool& reussite) {
    const RegleConway regle;

    Grille bloc = grilleVide(4, 4);
    bloc.modifierCellule(1, 1, true);
    bloc.modifierCellule(1, 2, true);
    bloc.modifierCellule(2, 1, true);
    bloc.modifierCellule(2, 2, true);
    Jeu jeu(bloc, regle);
    jeu.iterer();
    verifier(jeu.estStable(), "le bloc 2x2 doit être stable avant la bascule", reussite);

    jeu.basculerObstacle(1, 1);
    verifier(jeu.grille().estVivante(1, 1),
             "basculer un obstacle ne doit pas tuer une cellule vivante", reussite);
    verifier(jeu.grille().estObstacle(1, 1),
             "basculer l'obstacle d'une cellule vivante doit l'activer", reussite);
    verifier(!jeu.estStable(),
             "basculer l'obstacle d'une cellule vivante doit lever la stabilité",
             reussite);

    jeu.basculerObstacle(0, 0);
    verifier(!jeu.grille().estVivante(0, 0),
             "basculer un obstacle ne doit pas faire naître une cellule morte", reussite);
    verifier(jeu.grille().estObstacle(0, 0),
             "basculer l'obstacle d'une cellule morte doit l'activer", reussite);
    verifier(!jeu.estStable(),
             "basculer l'obstacle d'une cellule morte doit lever la stabilité",
             reussite);
}

void testerDoubleBasculeTorique(bool& reussite) {
    Grille grille = grilleAvec(2, 3, {{0, 1}, {1, 2}});
    grille.modifierObstacle(0, 0, true);
    grille.modifierObstacle(1, 1, true);
    const Grille reference = grille;

    grille.definirTorique(true);
    grille.definirTorique(false);
    verifier(!grille.estTorique(),
             "deux bascules du tore doivent revenir au mode non torique", reussite);
    verifier(memesCellulesEtObstacles(grille, reference),
             "deux bascules du tore ne doivent modifier ni états ni obstacles", reussite);
}

void testerReinitialisationEtEffacement(bool& reussite) {
    const RegleConway regle;

    // Le tore et les obstacles sont définis avant la construction du Jeu : c'est cet
    // état initial que reinitialiser doit restaurer.
    Grille initiale = grilleVide(4, 4);
    initiale.definirTorique(true);
    initiale.modifierCellule(1, 1, true);
    initiale.modifierObstacle(1, 1, true);
    initiale.modifierCellule(0, 3, true);
    initiale.modifierObstacle(3, 3, true);

    Jeu jeu(initiale, regle);
    verifier(jeu.grille().estTorique(),
             "le Jeu doit conserver le tore défini avant sa construction", reussite);
    verifier(memesCellulesEtObstacles(jeu.grille(), initiale),
             "le Jeu doit conserver les états et obstacles définis avant sa construction",
             reussite);

    jeu.definirTorique(false);
    verifier(!jeu.grille().estTorique(),
             "Jeu::definirTorique doit transmettre le mode à sa grille", reussite);
    jeu.basculerObstacle(3, 3);
    jeu.basculerObstacle(0, 0);
    jeu.modifierCellule(2, 2, true);
    jeu.iterer();
    jeu.reinitialiser();

    verifier(jeu.grille().estTorique(),
             "reinitialiser doit restaurer le mode torique initial", reussite);
    verifier(memesCellulesEtObstacles(jeu.grille(), initiale),
             "reinitialiser doit restaurer les états et obstacles définis avant la construction du Jeu",
             reussite);
    verifier(jeu.grille().estObstacle(3, 3) && !jeu.grille().estVivante(3, 3),
             "reinitialiser doit restaurer l'obstacle mort (3,3)", reussite);
    verifier(!jeu.grille().estObstacle(0, 0),
             "reinitialiser doit retirer l'obstacle ajouté après la construction", reussite);
    verifier(jeu.iteration() == 0,
             "reinitialiser doit remettre le compteur d'itérations à zéro", reussite);
    verifier(!jeu.estStable(),
             "reinitialiser doit laisser le jeu non stable", reussite);

    Grille pleine = grilleAvec(3, 3, {{0, 0}, {0, 1}, {1, 1}});
    pleine.definirTorique(true);
    pleine.modifierObstacle(0, 0, true);
    pleine.modifierObstacle(2, 2, true);
    Jeu jeuPlein(pleine, regle);
    jeuPlein.iterer();
    jeuPlein.effacer();

    verifier(jeuPlein.iteration() == 0,
             "effacer doit remettre le compteur d'itérations à zéro", reussite);
    verifier(!jeuPlein.estStable(),
             "effacer doit laisser le jeu non stable", reussite);
    verifier(grilleEntierementVide(jeuPlein.grille()),
             "effacer doit tuer toutes les cellules et retirer tous les obstacles",
             reussite);
}

void testerCopieDeGrille(bool& reussite) {
    // La copie de Grille exerce les copies de Cellule (construction et affectation)
    // sans accéder aux champs privés : états, obstacles et indépendance sont vérifiés.
    Grille source(2, 3, {1, 0, 0,
                         0, 0, 0});
    source.definirTorique(true);
    source.modifierObstacle(0, 0, true);
    source.modifierObstacle(1, 2, true);

    Grille copie(source);
    verifier(memesCellulesEtObstacles(copie, source),
             "la copie par construction doit conserver états et obstacles", reussite);
    verifier(copie.estTorique(),
             "la copie par construction doit conserver le mode torique", reussite);

    Grille affectee = grilleVide(2, 3);
    affectee = source;
    verifier(memesCellulesEtObstacles(affectee, source),
             "l'affectation de grille doit conserver états et obstacles", reussite);
    verifier(affectee.estTorique(),
             "l'affectation de grille doit conserver le mode torique", reussite);

    affectee.modifierCellule(0, 0, false);
    affectee.modifierObstacle(0, 0, false);
    verifier(source.estVivante(0, 0) && source.estObstacle(0, 0),
             "modifier une grille affectée ne doit pas modifier l'original", reussite);

    copie.modifierCellule(1, 1, true);
    copie.modifierObstacle(1, 1, true);
    verifier(!source.estVivante(1, 1) && !source.estObstacle(1, 1),
             "modifier la copie ne doit pas modifier l'original", reussite);

    source.modifierCellule(0, 0, false);
    source.modifierObstacle(0, 0, false);
    source.modifierObstacle(1, 2, false);
    verifier(copie.estVivante(0, 0) && copie.estObstacle(0, 0),
             "la copie doit conserver l'état vivant et l'obstacle après modification de l'original",
             reussite);
    verifier(copie.estObstacle(1, 2),
             "la copie doit conserver un obstacle mort après modification de l'original",
             reussite);
}

void testerAMemesEtats(bool& reussite) {
    // Contrat actuel : uniquement les dimensions et les états vivants/morts.
    const Grille base = grilleAvec(2, 3, {{0, 1}, {1, 2}});

    Grille memeEtats = base;
    memeEtats.definirTorique(true);
    memeEtats.modifierObstacle(0, 0, true);
    verifier(base.aMemesEtats(memeEtats),
             "aMemesEtats doit ignorer le tore et les obstacles", reussite);

    Grille etatDifferent = base;
    etatDifferent.basculerCellule(0, 1);
    verifier(!base.aMemesEtats(etatDifferent),
             "aMemesEtats doit être faux si un état diffère", reussite);

    Grille obstacleSeul = base;
    obstacleSeul.modifierObstacle(0, 1, true);
    verifier(base.aMemesEtats(obstacleSeul),
             "aMemesEtats doit rester vrai si seul un obstacle diffère", reussite);

    const Grille uneLigne(1, 6, {0, 1, 1, 0, 1, 1});
    const Grille troisLignes(3, 2, {0, 1,
                                    1, 0,
                                    1, 1});
    verifier(!uneLigne.aMemesEtats(troisLignes),
             "aMemesEtats doit tenir compte des dimensions", reussite);
}

void testerPetitesGrilles(bool& reussite) {
    const RegleConway regle;

    // 1x1 : les huit décalages se replient tous sur la cellule elle-même, qui est donc
    // sa propre voisine huit fois et meurt (règle de survie à 2 ou 3 voisines).
    Grille uneCellule = grilleAvec(1, 1, {{0, 0}});
    uneCellule.definirTorique(true);
    uneCellule.evoluer(regle);
    verifier(!uneCellule.estVivante(0, 0),
             "en tore 1x1, une cellule vivante a huit voisines repliées sur elle-même et doit mourir",
             reussite);

    // 1x3, une seule vivante en (0,0) : avec les répétitions modulo, la cellule (0,0)
    // est comptée deux fois comme sa propre voisine (lignes -1 et +1), donc deux
    // voisines et elle survit ; (0,1) et (0,2) voient (0,0) trois fois chacune,
    // donc trois voisines et elles naissent.
    Grille ligne = grilleAvec(1, 3, {{0, 0}});
    ligne.definirTorique(true);
    ligne.evoluer(regle);
    const Grille attendueLigne(1, 3, {1, 1, 1});
    verifier(memesCellulesEtObstacles(ligne, attendueLigne),
             "en tore 1x3, [1,0,0] doit donner [1,1,1] avec les voisinages repliés comptés modulo",
             reussite);

    // 1x3, deux vivantes adjacentes : chaque vivante a deux fois elle-même et trois
    // fois l'autre, soit cinq voisines, et meurt ; la morte voit chaque vivante trois
    // fois, soit six voisines, et reste morte.
    Grille ligneDeux = grilleAvec(1, 3, {{0, 0}, {0, 1}});
    ligneDeux.definirTorique(true);
    ligneDeux.evoluer(regle);
    const Grille attendueMorte(1, 3, {0, 0, 0});
    verifier(memesCellulesEtObstacles(ligneDeux, attendueMorte),
             "en tore 1x3, [1,1,0] doit donner [0,0,0] avec les répétitions modulo",
             reussite);
}

void testerGrandeGrille(bool& reussite) {
    const RegleConway regle;
    constexpr int cote = 200;
    constexpr int depart = 99;

    // Un planeur au centre d'une grille 200x200 emprunte le chemin parallèle.
    Grille grande = grilleVide(cote, cote);
    const std::pair<int, int> planeur[] = {
        {0, 1}, {1, 2}, {2, 0}, {2, 1}, {2, 2}};
    for (const auto& [ligne, colonne] : planeur) {
        grande.modifierCellule(depart + ligne, depart + colonne, true);
    }

    for (int numero = 0; numero < 4; ++numero) {
        grande.evoluer(regle);
    }

    // Après quatre itérations, le planeur est translaté d'une case en diagonale.
    Grille attendue = grilleVide(cote, cote);
    const std::pair<int, int> planeurTranslaté[] = {
        {1, 2}, {2, 3}, {3, 1}, {3, 2}, {3, 3}};
    for (const auto& [ligne, colonne] : planeurTranslaté) {
        attendue.modifierCellule(depart + ligne, depart + colonne, true);
    }

    verifier(grande.aMemesEtats(attendue),
             "un planeur évolue correctement sur une grille 200x200", reussite);
}

}

bool TestsExtensions::executer() {
    bool reussite = true;

    testerToreEtRepli(reussite);
    testerTore2x2Multiplicites(reussite);
    testerObstacles(reussite);
    testerObstacleDeBordTorique(reussite);
    testerAccesHorsBornes(reussite);
    testerBasculeEtStabilite(reussite);
    testerDoubleBasculeTorique(reussite);
    testerReinitialisationEtEffacement(reussite);
    testerCopieDeGrille(reussite);
    testerAMemesEtats(reussite);
    testerPetitesGrilles(reussite);
    testerGrandeGrille(reussite);

    if (reussite) {
        std::cout << "TestsExtensions : tous les tests sont passés.\n";
        return true;
    }
    return false;
}
