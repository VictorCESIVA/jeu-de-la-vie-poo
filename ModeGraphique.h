#pragma once

#include "Jeu.h"

#include <SFML/Graphics.hpp>

#include <string>
#include <vector>

class ModeGraphique {
public:
    explicit ModeGraphique(int delaiMs);
    void executer(Jeu& jeu, int nombreIterationsMaximum);

private:
    enum class Commande {
        LecturePause,
        Pas,
        VitesseMoins,
        VitessePlus,
        Reinitialiser,
        Effacer,
        Cellule,
        Motif
    };

    struct Bouton {
        sf::RectangleShape forme;
        sf::Text texte;
        Commande commande = Commande::Cellule;
        int indiceMotif = -1;
    };

    int delaiMs_;
    std::vector<Motif> motifs_;
    sf::Font police_;
    std::vector<Bouton> boutons_;
    int motifSelectionne_ = -1;  // -1 : outil cellule, sinon indice dans motifs_
    int nombreIterationsMaximum_ = 0;
    bool enCours_ = true;
    bool limiteAtteinte_ = false;
    float statutY_ = 0.0f;
    float hauteurInterface_ = 0.0f;

    void creerInterface(float largeurGrille);
    void ajouterBouton(const std::string& libelle, Commande commande,
                       int indiceMotif, float x, float largeur, float& y);
    void afficher(sf::RenderWindow& fenetre, const Jeu& jeu,
                  float tailleCellule, float largeurGrille);
    void traiterTouche(sf::Keyboard::Key touche, Jeu& jeu);
    bool traiterClicBouton(sf::Vector2f position, Jeu& jeu);
    void traiterClicGrille(sf::Vector2f position, Jeu& jeu, float tailleCellule);
    void basculerLecture(const Jeu& jeu);
    void avancer(Jeu& jeu);
    void reinitialiser(Jeu& jeu);
    void effacer(Jeu& jeu);
    void changerDelai(int variation);
    void selectionnerMotif(int indice);
};
