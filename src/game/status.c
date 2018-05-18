// <Insert project name here>
// Status (header)
// (c) 2018 Jani Nykänen

#include "status.h"

#include "../include/std.h"
#include "../include/renderer.h"

// Constants
static float HEALTH_FADE_MAX = 30.0f;
static int HEALTH_MAX = 3;
static int SCORE_BASE = 10;

// Status variables
static int health;
static float healthFadeTimer;
static bool healthFade;
static unsigned int score;
static int coins;

// Bitmap
static BITMAP* bmpHUD;
static BITMAP* bmpFont;
static BITMAP* bmpFontBig;


// Get score string
static void get_score_string(char* buf, int bufLen) {

    char zeroes[6];

    // Add zeroes
    int i = 4;
    int p = 0;
    for(; score < (unsigned int)pow(10, i); -- i ) {

        zeroes[p ++] = '0';
    }
    zeroes[p] = '\0';

    // Create score string
    if(score > 0)
        snprintf(buf, bufLen, "%s%d", zeroes, score);

    else
        snprintf(buf, bufLen, "%s", zeroes); 
}


// Initialize status
void init_status(ASSET_PACK* ass) {

    // Get assets
    bmpHUD = (BITMAP*)assets_get(ass, "HUD");
    bmpFont = (BITMAP*)assets_get(ass, "font");
    bmpFontBig = (BITMAP*)assets_get(ass, "fontBig");

    // (Re)set variables
    reset_status();
}


// Reset status
void reset_status() {

    health = HEALTH_MAX;
    healthFadeTimer = 0.0f;
    healthFade = false;
    score = 0;
    coins = 0;
}


// Update status
void status_update(float tm) {

    // Update fading health
    if(healthFade) {

        healthFadeTimer -= 1.0f * tm;
        if(healthFadeTimer <= 0.0f)
            healthFade = false;
    }
}


// Draw status
void status_draw() {

    const int HEART_X = 0;
    const int HEART_Y = 0;
    const int HEART_DELTA = 22;
    const int SCORE_TEXT_Y = 0;
    const int SCORE_Y = 4;
    const int COIN_TEXT_Y = -4;
    const int COIN_Y = 0;

    int i = 0;
    int sx = 0;
    char str[16];

    // Draw hearts
    for(; i < HEALTH_MAX; ++ i) {

        sx = health >= i ? 24 : 0;

        draw_bitmap_region(bmpHUD, sx,0,24,24, 
            HEART_X + HEART_DELTA*i, HEART_Y, 0 );
    }

    // Draw score
    get_score_string(str, 16);
    draw_text(bmpFont, " SCORE:", 128, SCORE_TEXT_Y, -6, 0, true);
    draw_text(bmpFontBig, str, 128, SCORE_Y, -16, 0, true);

    // Draw coins
    int coinX;
    if(coins < 10)
        coinX = 256-48;

    else if(coins < 100)
        coinX = 256-60;

    else
        coinX = 256-72;

    snprintf(str, 16, "~%d", coins);
    draw_text(bmpFontBig, str, coinX, COIN_TEXT_Y, -16, 0, false);
    draw_bitmap_region(bmpHUD,48,0,24,24, coinX -16, COIN_Y, 0);

}


// Reduce health
void status_reduce_health() {

    -- health;
    healthFadeTimer = HEALTH_FADE_MAX;
}


// Add a coin
void status_add_coin() {

    ++ coins;
}


// Add score
void status_add_score() {

    score += SCORE_BASE + coins;
}