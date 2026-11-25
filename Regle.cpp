#include "Regle.h"

bool RegleConway::doitEtreVivante(bool estActuellementVivante,
                                  int nombreVoisinesVivantes) const {
    if (estActuellementVivante) {
        return nombreVoisinesVivantes == 2 || nombreVoisinesVivantes == 3;
    }
    return nombreVoisinesVivantes == 3;
}
