#include <genesis.h>
#include "title_screen.h"
#include "game.h"

int main() {
    // Inicialización del sistema, configuración de paletas, etc.

    while (TRUE) {
        showTitleScreen(); // Mostrar la pantalla de título

        if (detectStartButton()) {
            startGame(); // Iniciar el juego principal si se presiona el botón Start
        }

        SYS_doVBlankProcess();
    }

    return 0;
}
