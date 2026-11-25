#pragma once

#include <memory>

class EtatCellule {
public:
    virtual ~EtatCellule() = default;

    virtual bool estVivante() const = 0;
    virtual std::unique_ptr<EtatCellule> cloner() const = 0;
};

class EtatVivant final : public EtatCellule {
public:
    bool estVivante() const override;
    std::unique_ptr<EtatCellule> cloner() const override;
};

class EtatMort final : public EtatCellule {
public:
    bool estVivante() const override;
    std::unique_ptr<EtatCellule> cloner() const override;
};
