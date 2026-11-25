#include "ModeGraphique.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
const float LARGEUR_BARRE = 250.0f;
const float MARGE = 10.0f;
const float HAUTEUR_BOUTON = 30.0f;
const float ESPACEMENT_BOUTON = 8.0f;
const float HAUTEUR_LIGNE_STATUT = 20.0f;
const char* CHEMIN_POLICE = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";

sf::String texteUtf8(const std::string& texte) {
    return sf::String::fromUtf8(texte.begin(), texte.end());
}
}

ModeGraphique::ModeGraphique(int delaiMs)
    : delaiMs_(delaiMs), motifs_(Motif::predefinis()) {
    if (delaiMs_ < 10) {
        throw std::invalid_argument("Le délai graphique doit être d'au moins 10 ms.");
    }

    if (!police_.loadFromFile(CHEMIN_POLICE)) {
        throw std::runtime_error("Impossible de charger la police " + std::string(CHEMIN_POLICE) +
                                 ". Vérifiez que le paquet fonts-dejavu est installé.");
    }
}

void ModeGraphique::creerInterface(float largeurGrille) {
    boutons_.clear();

    const float x = largeurGrille + MARGE;
    const float largeur = LARGEUR_BARRE - 2.0f * MARGE;
    float y = MARGE;

    ajouterBouton("Pause", Commande::LecturePause, -1, x, largeur, y);
    ajouterBouton("Pas à pas", Commande::Pas, -1, x, largeur, y);
    ajouterBouton("Vitesse -", Commande::VitesseMoins, -1, x, largeur, y);
    ajouterBouton("Vitesse +", Commande::VitessePlus, -1, x, largeur, y);
    ajouterBouton("Réinitialiser", Commande::Reinitialiser, -1, x, largeur, y);
    ajouterBouton("Effacer", Commande::Effacer, -1, x, largeur, y);
    ajouterBouton("Outil cellule", Commande::Cellule, -1, x, largeur, y);

    for (int indice = 0; indice < static_cast<int>(motifs_.size()); ++indice) {
        ajouterBouton(motifs_[static_cast<std::size_t>(indice)].nom(), Commande::Motif,
                      indice, x, largeur, y);
    }

    statutY_ = y;
    hauteurInterface_ = statutY_ + 4.0f * HAUTEUR_LIGNE_STATUT + MARGE;
}

void ModeGraphique::ajouterBouton(const std::string& libelle, Commande commande,
                                  int indiceMotif, float x, float largeur, float& y) {
    Bouton bouton;
    bouton.forme.setSize(sf::Vector2f(largeur, HAUTEUR_BOUTON));
    bouton.forme.setPosition(x, y);
    bouton.forme.setFillColor(sf::Color(60, 60, 60));
    bouton.forme.setOutlineThickness(1.0f);
    bouton.forme.setOutlineColor(sf::Color(110, 110, 110));
    bouton.texte.setFont(police_);
    bouton.texte.setString(texteUtf8(libelle));
    bouton.texte.setCharacterSize(14);
    bouton.texte.setFillColor(sf::Color::White);
    bouton.texte.setPosition(x + 8.0f, y + 7.0f);
    bouton.commande = commande;
    bouton.indiceMotif = indiceMotif;
    boutons_.push_back(bouton);

    y += HAUTEUR_BOUTON + ESPACEMENT_BOUTON;
}

void ModeGraphique::afficher(sf::RenderWindow& fenetre, const Jeu& jeu,
                             float tailleCellule, float largeurGrille) {
    fenetre.clear(sf::Color(35, 35, 35));

    sf::RectangleShape barre(
        sf::Vector2f(LARGEUR_BARRE, static_cast<float>(fenetre.getSize().y)));
    barre.setPosition(largeurGrille, 0.0f);
    barre.setFillColor(sf::Color(25, 25, 25));
    fenetre.draw(barre);

    sf::RectangleShape carre(sf::Vector2f(tailleCellule - 1.0f, tailleCellule - 1.0f));
    for (int ligne = 0; ligne < jeu.grille().lignes(); ++ligne) {
        for (int colonne = 0; colonne < jeu.grille().colonnes(); ++colonne) {
            carre.setPosition(static_cast<float>(colonne) * tailleCellule,
                              static_cast<float>(ligne) * tailleCellule);
            carre.setFillColor(jeu.grille().estVivante(ligne, colonne) ? sf::Color::White
                                                                      : sf::Color::Black);
            fenetre.draw(carre);
        }
    }

    for (Bouton& bouton : boutons_) {
        bool selectionne = false;
        if (bouton.commande == Commande::Cellule) {
            selectionne = motifSelectionne_ < 0;
        } else if (bouton.commande == Commande::Motif) {
            selectionne = bouton.indiceMotif == motifSelectionne_;
        }

        bouton.forme.setFillColor(selectionne ? sf::Color(45, 105, 170)
                                              : sf::Color(60, 60, 60));
        bouton.forme.setOutlineColor(selectionne ? sf::Color(140, 190, 240)
                                                 : sf::Color(110, 110, 110));
        if (bouton.commande == Commande::LecturePause) {
            bouton.texte.setString(texteUtf8(enCours_ ? "Pause" : "Lecture"));
        }
        fenetre.draw(bouton.forme);
        fenetre.draw(bouton.texte);
    }

    std::string etat = "En pause";
    if (jeu.estStable()) {
        etat = "Stable";
    } else if (limiteAtteinte_) {
        etat = "Limite atteinte";
    } else if (enCours_) {
        etat = "En cours";
    }

    const std::string outil = motifSelectionne_ < 0
                                  ? "Cellule"
                                  : motifs_[static_cast<std::size_t>(motifSelectionne_)].nom();
    const std::vector<std::string> lignes = {
        "État : " + etat,
        "Itération : " + std::to_string(jeu.iteration()),
        "Délai : " + std::to_string(delaiMs_) + " ms",
        "Outil : " + outil};

    sf::Text texte;
    texte.setFont(police_);
    texte.setCharacterSize(14);
    texte.setFillColor(sf::Color(220, 220, 220));
    float y = statutY_;
    for (const std::string& ligne : lignes) {
        texte.setString(texteUtf8(ligne));
        texte.setPosition(largeurGrille + MARGE, y);
        fenetre.draw(texte);
        y += HAUTEUR_LIGNE_STATUT;
    }

    fenetre.setTitle("Jeu de la vie | " + etat + " | Iteration " +
                     std::to_string(jeu.iteration()) + " | Delai " +
                     std::to_string(delaiMs_) + " ms");
    fenetre.display();
}

void ModeGraphique::basculerLecture(const Jeu& jeu) {
    if (!jeu.estStable() && !limiteAtteinte_) {
        enCours_ = !enCours_;
    }
}

void ModeGraphique::avancer(Jeu& jeu) {
    enCours_ = false;
    if (jeu.estStable() || jeu.iteration() >= nombreIterationsMaximum_) {
        return;
    }

    jeu.iterer();
    if (jeu.iteration() >= nombreIterationsMaximum_) {
        limiteAtteinte_ = true;
    }
}

void ModeGraphique::reinitialiser(Jeu& jeu) {
    jeu.reinitialiser();
    enCours_ = true;
    limiteAtteinte_ = false;
}

void ModeGraphique::effacer(Jeu& jeu) {
    jeu.effacer();
    enCours_ = false;
    limiteAtteinte_ = false;
}

void ModeGraphique::changerDelai(int variation) {
    delaiMs_ = std::min(2000, std::max(10, delaiMs_ + variation));
}

void ModeGraphique::selectionnerMotif(int indice) {
    if (indice < 0 || indice >= static_cast<int>(motifs_.size())) {
        return;
    }
    motifSelectionne_ = indice;
}

void ModeGraphique::traiterTouche(sf::Keyboard::Key touche, Jeu& jeu) {
    switch (touche) {
    case sf::Keyboard::Space:
        basculerLecture(jeu);
        break;
    case sf::Keyboard::N:
    case sf::Keyboard::Right:
        avancer(jeu);
        break;
    case sf::Keyboard::Add:
    case sf::Keyboard::Equal:
        changerDelai(-50);
        break;
    case sf::Keyboard::Subtract:
    case sf::Keyboard::Hyphen:
        changerDelai(50);
        break;
    case sf::Keyboard::R:
        reinitialiser(jeu);
        break;
    case sf::Keyboard::C:
        effacer(jeu);
        break;
    case sf::Keyboard::Num0:
    case sf::Keyboard::Numpad0:
        motifSelectionne_ = -1;
        break;
    case sf::Keyboard::Num1:
    case sf::Keyboard::Numpad1:
        selectionnerMotif(0);
        break;
    case sf::Keyboard::Num2:
    case sf::Keyboard::Numpad2:
        selectionnerMotif(1);
        break;
    case sf::Keyboard::Num3:
    case sf::Keyboard::Numpad3:
        selectionnerMotif(2);
        break;
    case sf::Keyboard::Num4:
    case sf::Keyboard::Numpad4:
        selectionnerMotif(3);
        break;
    default:
        break;
    }
}

bool ModeGraphique::traiterClicBouton(sf::Vector2f position, Jeu& jeu) {
    for (const Bouton& bouton : boutons_) {
        if (!bouton.forme.getGlobalBounds().contains(position)) {
            continue;
        }

        switch (bouton.commande) {
        case Commande::LecturePause:
            basculerLecture(jeu);
            break;
        case Commande::Pas:
            avancer(jeu);
            break;
        case Commande::VitesseMoins:
            changerDelai(50);
            break;
        case Commande::VitessePlus:
            changerDelai(-50);
            break;
        case Commande::Reinitialiser:
            reinitialiser(jeu);
            break;
        case Commande::Effacer:
            effacer(jeu);
            break;
        case Commande::Cellule:
            motifSelectionne_ = -1;
            break;
        case Commande::Motif:
            selectionnerMotif(bouton.indiceMotif);
            break;
        }
        return true;
    }

    return false;
}

void ModeGraphique::traiterClicGrille(sf::Vector2f position, Jeu& jeu,
                                      float tailleCellule) {
    if (position.x < 0.0f || position.y < 0.0f) {
        return;
    }

    const int colonne = static_cast<int>(position.x / tailleCellule);
    const int ligne = static_cast<int>(position.y / tailleCellule);
    if (ligne >= jeu.grille().lignes() || colonne >= jeu.grille().colonnes()) {
        return;
    }

    bool modifie = false;
    if (motifSelectionne_ < 0) {
        jeu.basculerCellule(ligne, colonne);
        modifie = true;
    } else {
        modifie = jeu.placerMotif(motifs_[static_cast<std::size_t>(motifSelectionne_)],
                                  ligne, colonne);
    }

    if (modifie) {
        enCours_ = false;
    }
}

void ModeGraphique::executer(Jeu& jeu, int nombreIterationsMaximum) {
    nombreIterationsMaximum_ = nombreIterationsMaximum;

    const float tailleCellule = std::max(
        4.0f, std::min(20.0f, 800.0f / std::max(jeu.grille().lignes(),
                                               jeu.grille().colonnes())));
    const float largeurGrille = static_cast<float>(jeu.grille().colonnes()) * tailleCellule;
    const float hauteurGrille = static_cast<float>(jeu.grille().lignes()) * tailleCellule;
    creerInterface(largeurGrille);

    const unsigned int largeur =
        static_cast<unsigned int>(largeurGrille + LARGEUR_BARRE);
    const unsigned int hauteur =
        static_cast<unsigned int>(std::max(hauteurGrille, hauteurInterface_));

    sf::RenderWindow fenetre(sf::VideoMode(largeur, hauteur), "Jeu de la vie",
                             sf::Style::Titlebar | sf::Style::Close);
    fenetre.setFramerateLimit(60);
    sf::Clock horloge;

    while (fenetre.isOpen()) {
        sf::Event evenement;
        while (fenetre.pollEvent(evenement)) {
            if (evenement.type == sf::Event::Closed) {
                fenetre.close();
            } else if (evenement.type == sf::Event::KeyPressed) {
                traiterTouche(evenement.key.code, jeu);
            } else if (evenement.type == sf::Event::MouseButtonPressed &&
                       evenement.mouseButton.button == sf::Mouse::Left) {
                const sf::Vector2f position(
                    static_cast<float>(evenement.mouseButton.x),
                    static_cast<float>(evenement.mouseButton.y));
                if (!traiterClicBouton(position, jeu)) {
                    traiterClicGrille(position, jeu, tailleCellule);
                }
            }
        }

        if (enCours_ && !jeu.estStable() &&
            jeu.iteration() < nombreIterationsMaximum_ &&
            horloge.getElapsedTime().asMilliseconds() >= delaiMs_) {
            jeu.iterer();
            horloge.restart();
        }

        if (jeu.estStable()) {
            enCours_ = false;
        }
        if (jeu.iteration() >= nombreIterationsMaximum_) {
            limiteAtteinte_ = true;
            enCours_ = false;
        }

        afficher(fenetre, jeu, tailleCellule, largeurGrille);
    }

    std::cout << "Mode graphique terminé à l'itération " << jeu.iteration() << ".\n";
}
