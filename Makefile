CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pedantic -pthread
LDLIBS = -lsfml-graphics -lsfml-window -lsfml-system

# PREFIX=/chemin/vers/sfml pour une installation locale, sans sudo.
PREFIX ?=
ifneq ($(PREFIX),)
CXXFLAGS += -I$(PREFIX)/include
LDFLAGS += -L$(PREFIX)/lib -Wl,--disable-new-dtags -Wl,-rpath,$(PREFIX)/lib
endif

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
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@ $(LDFLAGS) $(LDLIBS)

test: $(CIBLE)
	./$(CIBLE) --test

clean:
	$(RM) $(CIBLE)
