#include <genesis.h>
#include "title_screen.h"
#include "game.h" // Necesitas incluir game.h para poder acceder a la función que inicia el juego principal
#include "resources.h" // Incluye recursos externos, como gráficos y paletas

#define SPRITE_VISIBILITY_DELAY 64 // 0.64 segundos a 60 FPS

Sprite *pressStartSprite;
Sprite *mermaidspr;

void showTitleScreen() {
    // Configurar la paleta para el fondo y los sprites
    PAL_setPalette(PAL0, bgt_pal.data, CPU);
    PAL_setPalette(PAL1, ps_pal.data, CPU);
    PAL_setPalette(PAL2, m_pal.data, CPU);

    // Dibujar la imagen de fondo en la pantalla
    VDP_setBackgroundColor(0); // Establece el color de fondo
    VDP_setScreenWidth320();   // Establece la resolución de pantalla a 320 píxeles de ancho
    int ind = TILE_USER_INDEX;
    VDP_drawImageEx(BG_B, &bgtitle, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);


    // Añadir el sprite de "Press Start" al centro de la pantalla
    pressStartSprite = SPR_addSprite(&mermaid, 120, 200, TILE_ATTR(PAL2, 0, FALSE, FALSE));
    int visibilityCounter = 0;
    bool isVisible = TRUE;

    while (!detectStartButton()) {
        // Incrementar el contador de visibilidad
        visibilityCounter++;
        if (visibilityCounter >= SPRITE_VISIBILITY_DELAY) {
            // Cambiar la visibilidad del sprite
            isVisible = !isVisible;
            SPR_setVisibility(pressStartSprite, isVisible);
            visibilityCounter = 0; // Reiniciar el contador
        }

        // Actualizar los sprites en la pantalla después de cambiar la visibilidad del sprite de "Press Start"
        SPR_update();

        // Aquí puedes agregar cualquier lógica adicional para la pantalla de título, como animaciones, etc.

        SYS_doVBlankProcess();
    }

    // Eliminar el sprite de "Press Start" cuando se detecte que se ha presionado el botón Start
    SPR_releaseSprite(pressStartSprite);
}


bool detectStartButton() {
    // Aquí detectas si el usuario ha presionado el botón Start para iniciar el juego
    return JOY_readJoypad(JOY_1) & BUTTON_START;
}