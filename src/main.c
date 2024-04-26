#include <genesis.h>   // Incluye las definiciones y funciones de la librería SGDK
#include "resources.h" // Incluye recursos externos, como gráficos y paletas

// Definiciones de constantes para el juego
#define TOTAL_LINES 210
#define FAR_N 80
#define LAND_LINES TOTAL_LINES - FAR_N
#define WAVE_LINES 90
#define LAND_LIMIT 218
#define MAX_BULLETS 10 // Número máximo de balas en pantalla
#define BULLET_SPEED 4 // Velocidad de las balas
#define BULLET_LIFETIME 180 // 3 segundos a 60 FPS
#define BULLET_COOLDOWN 20 // Número de frames entre disparos

// Variables para controlar el sprite del jugador
Sprite *player;
s16 player_spd_x; // Velocidad horizontal del jugador
s16 player_spd_y; // Velocidad vertical del jugador
s16 player_x;     // Posición horizontal del jugador
s16 player_y;     // Posición vertical del jugador

Sprite *bullets[MAX_BULLETS];    // Arreglo de sprites para las balas
bool bullet_active[MAX_BULLETS]; // Estado activo de cada bala
int bullet_lifetime[MAX_BULLETS]; // Tiempo de vida de cada bala
int bullet_cooldown = 0;          // Cooldown para disparar balas


#define SPEED 2 // Velocidad constante del jugador

// Declaración de funciones estáticas para manejo de entradas y físicas
static void updateInput();
static void updatePhysics();
static void updateBullets();
static void fireBullet();

int main(bool hard)
{
  // Configuración inicial de la pantalla y paleta de colores
  VDP_setBackgroundColor(0); // Establece el color de fondo
  VDP_setScreenWidth320();   // Establece la resolución de pantalla a 320 píxeles de ancho

  // Configura las paletas de colores para el fondo y el sprite del jugador
  PAL_setPalette(PAL0, bg_pal.data, CPU);
  PAL_setPalette(PAL1, m_pal.data, CPU);
  PAL_setPalette(PAL2, b_pal.data, CPU);

  // Configuración inicial de los tiles para el fondo
  int ind = TILE_USER_INDEX;
  VDP_drawImageEx(BG_B, &bg, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
  ind += bg.tileset->numTile;

  // Establece el modo de desplazamiento del fondo
  VDP_setScrollingMode(HSCROLL_LINE, VSCROLL_PLANE);

  // Prepara un array para el desplazamiento horizontal con efecto de onda
  s16 hScroll[TOTAL_LINES];
  memset(hScroll, 0, sizeof(hScroll)); // Inicializa el array con ceros

  // Configura el efecto de onda para el desplazamiento del fondo
  u16 sinPerLine = 4;
  fix16 amplitude = FIX16(16.0);
  s16 offset = -40;

  // Calcula el desplazamiento horizontal por línea para simular ondas
  for (u16 i = 0; i < TOTAL_LINES; ++i)
  {
    hScroll[i] = fix16ToInt(fix16Mul(sinFix16(i * sinPerLine), amplitude)) + offset;
  }

  // Configura el desplazamiento del "suelo" para simular perspectiva
  fix16 landOffsets[LAND_LINES];
  memset(landOffsets, 0, sizeof(landOffsets));

  // Inicialización del sistema de sprites
  SPR_init();

  // Añade el sprite del jugador a la pantalla
  player = SPR_addSprite(&mermaid, 100, 100, TILE_ATTR(PAL1, 0, FALSE, FALSE));

  // Inicializa las balas y su estado
  for (int i = 0; i < MAX_BULLETS; i++)
  {
    bullets[i] = SPR_addSprite(&bala, -32, -10, TILE_ATTR(PAL2, 0, FALSE, FALSE));
    bullet_active[i] = FALSE;
  }

  // Bucle principal del juego
  fix16 backgroundOffset = FIX16(0); // Desplazamiento constante del fondo
  fix16 speed_background = FIX16(-0.2);
  u16 sinOffset = 0; // Desplazamiento ondulatorio que cambia en cada frame
  while (TRUE)
  {
    updateInput();
    updatePhysics();
    updateBullets();

    if (bullet_cooldown > 0) bullet_cooldown--;

    backgroundOffset = fix16Add(backgroundOffset, speed_background);

    // Actualización del desplazamiento de onda con una fase variable
    sinOffset++;
    for (u16 i = 0; i < TOTAL_LINES; ++i) // Ahora aplicamos la onda a todas las líneas
    {
      s16 waveEffect = fix16ToInt(fix16Mul(sinFix16((i + sinOffset) * sinPerLine), amplitude));
      hScroll[i] = waveEffect + offset + fix16ToInt(backgroundOffset);
    }

    // Si deseas mantener el desplazamiento específico para el "suelo" también
    fix16 delta = FIX16(0.05);
    fix16 landOffset = FIX16(0.05); // Comienza con un pequeño desplazamiento
    for (u16 i = FAR_N; i < LAND_LIMIT; i++)
    {
      landOffset = fix16Add(landOffset, delta);
      landOffsets[i - FAR_N] = fix16Sub(landOffsets[i - FAR_N], landOffset);
      hScroll[i] = fix16ToInt(landOffsets[i - FAR_N]) + offset + fix16ToInt(backgroundOffset);
    }

    VDP_setHorizontalScrollLine(BG_B, 0, hScroll, TOTAL_LINES, DMA_QUEUE);
    SYS_doVBlankProcess();
    SPR_update();
  }
  return 0;
}


static void fireBullet()
{
  if (bullet_cooldown > 0) return; // No hacer nada si el enfriamiento no ha terminado

  for (int i = 0; i < MAX_BULLETS; i++)
  {
    if (!bullet_active[i])
    {
      bullet_active[i] = TRUE;
      bullet_lifetime[i] = BULLET_LIFETIME;
      SPR_setPosition(bullets[i], player_x + 54, player_y); // Ajusta la posición de inicio según necesites
      bullet_cooldown = BULLET_COOLDOWN; // Reinicia el contador de enfriamiento
      break;
    }
  }
}

// Función para actualizar la posición de las balas
static void SPR_getPosition(Sprite *sprite, s16 *x, s16 *y)
{
  *x = SPR_getPositionX(sprite);
  *y = SPR_getPositionY(sprite);
}


static void updateBullets()
{
  for (int i = 0; i < MAX_BULLETS; i++)
  {
    if (bullet_active[i])
    {
      s16 x, y;
      SPR_getPosition(bullets[i], &x, &y);
      x += BULLET_SPEED;
      if (x > 320)
      { // Asumiendo una resolución de 320 de ancho
        bullet_active[i] = FALSE;
        SPR_setPosition(bullets[i], -32, -10); // Mueve la bala fuera de la pantalla
      }
      else
      {
        SPR_setPosition(bullets[i], x, y);
      }
    }
  }
}

// Función para leer y procesar la entrada del jugador
static void updateInput()
{
  // Lee el estado actual del control
  u16 value = JOY_readJoypad(JOY_1);

  // Controla la dirección horizontal del jugador
  if (value & BUTTON_LEFT)
  {
    player_spd_x = -SPEED;
  }
  else if (value & BUTTON_RIGHT)
  {
    player_spd_x = SPEED;
  }
  else
  {
    player_spd_x = 0;
  }

  // Controla la dirección vertical del jugador
  if (value & BUTTON_UP)
  {
    player_spd_y = -SPEED;
  }
  else if (value & BUTTON_DOWN)
  {
    player_spd_y = SPEED;
  }
  else
  {
    player_spd_y = 0;
  }

  // Dispara un proyectil si el botón A está presionado
  if (value & BUTTON_A)
  {
    fireBullet();
  }
}

// Función para actualizar la física del jugador
static void updatePhysics()
{
  // Actualiza la posición del jugador basándose en su velocidad
  player_y += player_spd_y;
  player_x += player_spd_x;

  // Establece la nueva posición del sprite del jugador
  SPR_setPosition(player, player_x, player_y);
}
