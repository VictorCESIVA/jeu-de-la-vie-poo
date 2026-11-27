#pragma once

class Regle {
public:
    virtual ~Regle() = default;

    // Peut être appelée simultanément depuis plusieurs threads lors d'une évolution
    // parallèle : une implémentation doit être thread-safe et sans effet de bord.
    virtual bool doitEtreVivante(bool estActuellementVivante,
                                 int nombreVoisinesVivantes) const = 0;
};

class RegleConway final : public Regle {
public:
    bool doitEtreVivante(bool estActuellementVivante,
                         int nombreVoisinesVivantes) const override;
};
