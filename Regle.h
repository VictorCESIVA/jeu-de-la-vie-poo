#pragma once

class Regle {
public:
    virtual ~Regle() = default;

    virtual bool doitEtreVivante(bool estActuellementVivante,
                                 int nombreVoisinesVivantes) const = 0;
};

class RegleConway final : public Regle {
public:
    bool doitEtreVivante(bool estActuellementVivante,
                         int nombreVoisinesVivantes) const override;
};
