#pragma once
#include <string>
#include <vector>

class Motif {
    std::string nom_;
    std::vector<std::vector<int>> cellules_;
public:
    Motif(std::string nom, const std::vector<std::vector<int>>& cellules);
    const std::string& nom() const;
    int lignes() const;
    int colonnes() const;
    bool estVivante(int ligne, int colonne) const;
    static std::vector<Motif> predefinis();
};
