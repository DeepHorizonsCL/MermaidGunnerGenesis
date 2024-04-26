#include <genesis.h>
#include "resources.h"

#define TOTAL_LINES 210 
#define LAND_LINES TOTAL_LINES - FAR
#define FAR 80 
#define WAVE_LINES 90
#define LAND_LIMIT 218

Sprite* player;
s16 player_spd_x;
s16 player_spd_y;
s16 player_x;
s16 player_y;


#define SPEED	2



// Functions
static void updateInput();
static void updatePhysics();

int main(bool hard) {

  //////////////////////////////////////////////////////////////
  // setup screen and palettes
  VDP_setBackgroundColor(16);
  VDP_setScreenWidth320();

  PAL_setPalette( PAL0, bg_pal.data, CPU );
  PAL_setPalette( PAL1, m_pal.data, CPU );

  //////////////////////////////////////////////////////////////
  // setup scrolling
  int ind = TILE_USER_INDEX;
  VDP_drawImageEx(BG_B, &bg, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
  ind += bg.tileset->numTile;

  // use LINE scroll for horizontal 
  VDP_setScrollingMode( HSCROLL_LINE, VSCROLL_PLANE);

  s16 hScroll[TOTAL_LINES]; 
  memset( hScroll, 0, sizeof(hScroll));

  u16 sinPerLine = 4;    // Elements to jump in sin() per line. Larger values give us faster waves.
  fix16 amplitude = FIX16( 16.0 );  // Amplitude sets how big the waves are.
  s16 offset = -40;  // shift left a bit.
  for( u16 i = 0; i < TOTAL_LINES; ++i ) {
    hScroll[i] = fix16ToInt( fix16Mul(  sinFix16(i * sinPerLine ), amplitude ) ) + offset;
  }


  fix16 landOffsets[LAND_LINES];
  memset( landOffsets, 0, sizeof(landOffsets));

  SPR_init();

  player = SPR_addSprite(&mermaid,100,100,TILE_ATTR(PAL1,0, FALSE,FALSE));
  Sprite* SPR_addSprite(const SpriteDefinition * 	spriteDef, s16 x, s16 y, u16 attribute);

  //////////////////////////////////////////////////////////////
  // main loop.
  u16 sinOffset = 0; // Step basically tells us where we're starting in the sin table.
  while(TRUE)
  {
    // read joypad to set sinewave parameters 
    updateInput();
    updatePhysics();

    // write params to the screen.
    char message[40];
    char amps[5];
    fix16ToStr( amplitude, amps, 1 );
    strclr(message);
    sprintf( message, "sin per line: %d amplitude: %s  ", sinPerLine, amps );
    //DP_drawText(message, 3, 1 );

    //////////////////////////////////////////////////////////////
    // This is what matters right here:  
    //    calculate the offsets per line using SGDK's sin table
    //    and adjust with params
    sinOffset++; // move up in the sine table
    for( u16 i = 0; i < WAVE_LINES; ++i ) {
        // compute horizontal offsets with sine table.
        hScroll[i] = fix16ToInt( fix16Mul(  sinFix16(( i + sinOffset ) * sinPerLine ), amplitude ) ) + offset;
    }

    // scroll the land.  
    
    fix16 delta = FIX16(0.05);
    fix16 offset = FIX16(0.05);
    for (u16 i = FAR; i < LAND_LIMIT; i ++) {
      // increase the amount we scroll as we get closer to the bottom of the screen.
      offset = fix16Add( offset, delta);
      landOffsets[i-FAR] = fix16Sub( landOffsets[i-FAR], offset);
      hScroll[i] = fix16ToInt(landOffsets[i-FAR]);
    }
    
    // apply scrolling offsets 
    VDP_setHorizontalScrollLine (BG_B, 0, hScroll, 223, DMA_QUEUE);
    SYS_doVBlankProcess();
    
    SPR_update();
  }
  return 0;

}

static void updateInput()
{
	// Joypad Value
	u16 value = JOY_readJoypad(JOY_1);

	// Move Left/Right
	if(value & BUTTON_LEFT){ player_spd_x = -SPEED; }
	else if(value & BUTTON_RIGHT){ player_spd_x = SPEED; }
	else{ player_spd_x = 0; }

	// Move Up/Down
	if(value & BUTTON_UP){ player_spd_y = -SPEED; }
	else if(value & BUTTON_DOWN){ player_spd_y = SPEED; }
	else{ player_spd_y = 0; }


  /*
    if( joypad & BUTTON_A) {
      sinPerLine = 5;    
      amplitude = FIX16( 10.0 );
    } 
    if( joypad & BUTTON_B) {
      sinPerLine = 10;    
      amplitude = FIX16( 30.0 );
    } 
    if( joypad & BUTTON_C) {
      sinPerLine = 80;     
      amplitude = FIX16( 0.5 );
    } */
}

static void updatePhysics()
{

  player_y += player_spd_y; 
  player_x += player_spd_x;

  SPR_setPosition(player, player_x, player_y);
}