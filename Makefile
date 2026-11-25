CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
LDLIBS = -lsfml-graphics -lsfml-window -lsfml-system

CIBLE = jeu_de_la_vie
SOURCES = main.cpp EtatCellule.cpp Cellule.cpp Regle.cpp Motif.cpp Grille.cpp Jeu.cpp \
          FichierGrille.cpp ModeConsole.cpp ModeGraphique.cpp Tests.cpp TestsFichiers.cpp \
          TestsExtensions.cpp
ENTETES = EtatCellule.h Cellule.h Regle.h Motif.h Grille.h Jeu.h \
          FichierGrille.h ModeConsole.h ModeGraphique.h Tests.h TestsFichiers.h \
          TestsExtensions.h

.PHONY: all test clean

all: $(CIBLE)

$(CIBLE): $(SOURCES) $(ENTETES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@ $(LDLIBS)

test: $(CIBLE)
	./$(CIBLE) --test

clean:
	$(RM) $(CIBLE)
