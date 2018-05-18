// <Insert project name here>
// Monster (source)
// (c) 2018 Jani Nykänen

#include "monster.h"

#include "camera.h"

#include "../include/std.h"

// Constants
static const float DEATH_MAX = 20.0f;
static const float WALKER_SPEED = 0.5f;
static const float DEATH_GRAVITY = 0.1f;
static const float DEATH_SPEED_Y = -1.5f;
static const float DEATH_SPEED_X_MUL = 1.25f;

// Bitmaps
static BITMAP* bmpMonsters;
static BITMAP* bmpSplash;


// "Set monster"
static void set_monster(MONSTER* m) {

    switch(m->id) {

    case 0:
        m->direction = rand() % 2 == 0 ? 1 : -1;
        m->speed.x = WALKER_SPEED * m->direction;
        
        break;

    default:
        break;
    }

}


// Move monster
static void move_monster(MONSTER* m, float tm) {

    // Move axes
    m->pos.x += m->speed.x * tm;
    m->pos.y += m->speed.y * tm;

    // Limit collisions
    if(m->speed.x < 0.0f && m->pos.x-8.0f < m->leftLimit) {

        m->speed.x *= -1;
        m->pos.x = m->leftLimit +8.0f;
    }
    else if(m->speed.x > 0.0f && m->pos.x+8.0f > m->rightLimit) {

        m->speed.x *= -1;
        m->pos.x = m->rightLimit -8.0f;
    }

    // If "too high", die
    if(m->pos.y+1 < get_global_camera()->pos.y) {

        m->exist = false;
        m->deathTimer = -1.0f;
    }
}


// Animate
static void animate_monster(MONSTER* m, float tm) {

    switch(m->id) {

    case 0:
        spr_animate(&m->spr, m->id,0,3, 6, tm);
        m->flip = m->speed.x > 0.0f ? FLIP_H : FLIP_NONE;
        break;

    default:
        break;
    }
}


// Draw fading
static void draw_fading(int sx, int sy, int x, int y, int fade, int flip) {

    draw_bitmap_region_fading(bmpMonsters,sx,sy,
                32,32,x,y, flip, fade, get_alpha());
}


// Splash!
static void do_splash(MONSTER* m, float x, float y) {

    m->splashPos = vec2(x, y);

    m->splash.count = 0;
    m->splash.frame = 0;
}


// Initialize monsters
void init_monsters(ASSET_PACK* ass) {

    // Get assets
    bmpMonsters = (BITMAP*)assets_get(ass, "monsters");
    bmpSplash = (BITMAP*)assets_get(ass, "splash");
}


// Create a monster
MONSTER create_monster(VEC2 pos, float left, float right, int id) {

    MONSTER m;
    m.pos = pos;
    m.id = id;
    m.deathTimer = 0.0f;
    m.speed = vec2(0, 0);
    m.spr = create_sprite(32, 32);
    m.splash = create_sprite(32, 32);
    m.exist = true;
    m.dying = false;
    m.flip = FLIP_NONE;
    m.leftLimit = left;
    m.rightLimit = right;
    if(m.rightLimit > 256)
        m.rightLimit = 256;

    // Set id-depending values
    set_monster(&m);

    return m;
}


// Update a monster
void monster_update(MONSTER* m, float tm) {

    if(m->exist == false) {

        if(!m->dying) return;

        // Update death timer
        if(m->deathTimer > 0.0f) {

            m->deathTimer -= 1.0f * tm;

            m->pos.x += m->deathSpeed.x *tm;
            m->pos.y += m->deathSpeed.y *tm;

            if(!m->stomped)
                m->deathSpeed.y += DEATH_GRAVITY * tm;
        }

        // Animate splash
        if(m->splash.frame < 5)
            spr_animate(&m->splash, 0,0,5,5, tm);

        // If splash ended & death timer <= 0, no more dying
        if(m->splash.frame >= 5 && m->deathTimer <= 0.0f)
            m->dying = false;

        return;
    }

    // Move
    move_monster(m, tm);

    // Animate
    animate_monster(m, tm);
}


// Draw a monster
void monster_draw(MONSTER* m) {

    // Rendering position
    int x = (int)floorf(m->pos.x-16.0f);
    int y = (int)floorf(m->pos.y-28.0f) +1;

    // Splash position
    int sx = (int)floorf(m->splashPos.x-16.0f);
    int sy = (int)floorf(m->splashPos.y-16.0f);

    if(!m->exist) {
        
        if(!m->dying) return;

        // Fading
        if(m->deathTimer > 0.0f) {

            int fade = 1+ (int)roundf(m->deathTimer / DEATH_MAX * 8.0f);
            draw_fading(0, m->id,x,y,fade, m->flip);

        }   

        // Splash
        spr_draw(&m->splash, bmpSplash, sx, sy, FLIP_NONE);

        return;
    }

    // Draw
    spr_draw(&m->spr,bmpMonsters, x,y, m->flip);
}


// Monster-to-goat collision
void monster_goat_collision(MONSTER* m, GOAT* g) {

    const float GOAT_JUMP_BACK = -2.0f;

    if(!m->exist) return;

    // If in the same horizontal area
    if(g->pos.x+8.0f >= m->pos.x-8.0f && g->pos.x-8.0f <= m->pos.x+8.0f) {

        // If the goat is jumping over the monster
        if(g->speed.y > 0.0f && g->pos.y > m->pos.y-26.0f && g->pos.y < m->pos.y-16.0f) {

            m->exist = false;
            m->deathTimer = DEATH_MAX;
            m->deathSpeed = vec2(0, 0);
            m->dying = true;
            m->stomped = true;

            g->speed.y = GOAT_JUMP_BACK;

            do_splash(m, g->pos.x, m->pos.y-18);

            return;
        }

    }

    // Ram collision
    if(g->dashing) {

        // If coming from the correct direction
        if( (g->speed.x > 0.0f && g->pos.x < m->pos.x) 
        || (g->speed.x < 0.0f && g->pos.x > m->pos.x)) {


            // If inside the collision area
            if(g->pos.x+12.0f >= m->pos.x-8.0f && g->pos.x-12.0f <= m->pos.x+8.0f
            && g->pos.y >= m->pos.y-16 && g->pos.y-16.0f <= m->pos.y ) {

                m->exist = false;
                m->deathTimer = DEATH_MAX;
                m->dying = true;
                m->stomped = false;

                m->deathSpeed.x = g->speed.x * DEATH_SPEED_X_MUL;
                m->deathSpeed.y = DEATH_SPEED_Y;

                g->speed.x = 0.0f;
                goat_stop_dashing(g);
                
                do_splash(m,m->pos.x, g->pos.y -10.0f);

                return;
            }

        }
    }

    // Hurt collision
    goat_hurt_collision(g, m->pos.x-8.0f,m->pos.y-16.0f,16.0f,16.0f);
}
