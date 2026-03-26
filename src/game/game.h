#ifndef TD_GAME_H
#define TD_GAME_H

#include <stdbool.h>
#include "map.h"
#include "path.h"
#include "enemy.h"
#include "wave.h"
#include "tower.h"
#include "projectile.h"
#include "economy.h"
#include "level.h"
#include "particles.h"
#include "../render/camera.h"
#include "../render/sprite_batch.h"
#include "../render/debug_draw.h"
#include "../render/atlas_gen.h"
#include "../core/input.h"
#include "../core/td_time.h"
#include "../ui/ui.h"
#include "../audio/audio.h"

typedef enum Difficulty {
    DIFF_RECRUIT = 0,      /* "Fresh Recruit" — easy */
    DIFF_REGULAR,          /* "Regular Tommy" — normal */
    DIFF_VETERAN,          /* "Trench Veteran" — hard */
    DIFF_KAISERSCHLACHT,   /* "Kaiserschlacht" — legendary */
    DIFF_COUNT
} Difficulty;

/* Artillery barrage call-in ability */
#define BARRAGE_COOLDOWN_BASE   30.0f
#define BARRAGE_SHELLS          6
#define BARRAGE_SHELL_DELAY     0.25f
#define BARRAGE_RADIUS          2.5f
#define BARRAGE_DAMAGE          150.0f
#define BARRAGE_SPLASH          2.0f

typedef struct Barrage {
    float cooldown;         /* seconds remaining until ready */
    float cooldown_max;     /* total cooldown for current difficulty */
    bool  targeting;        /* player is choosing where to drop */
    Vec2  target;           /* world position of strike */
    bool  active;           /* strike in progress */
    float strike_timer;     /* time into the strike sequence */
    int   shells_dropped;   /* how many shells have landed so far */
} Barrage;

/* Trench whistle rally — slows all enemies on map */
#define RALLY_COOLDOWN_BASE     45.0f
#define RALLY_SLOW_FACTOR       0.35f   /* enemies move at 35% speed */
#define RALLY_SLOW_DURATION     4.0f    /* seconds */

typedef struct Rally {
    float cooldown;         /* seconds remaining until ready */
    float cooldown_max;     /* total cooldown for current difficulty */
    bool  active;           /* visual/audio feedback in progress */
    float effect_timer;     /* remaining time of visual flash */
} Rally;

typedef struct GameState {
    Map map;
    PathSet paths;
    EnemyManager enemies;
    WaveManager waves;
    TowerManager towers;
    ProjectileManager projectiles;
    Economy economy;
    ParticleSystem particles;
    Camera camera;
    SpriteBatch batch;
    DebugDraw debug_draw;
    GameAtlas atlas;
    LevelDef level_def;
    Barrage barrage;
    Rally rally;

    TowerType selected_tower;
    IVec2 hover_tile;
    int selected_tower_index; /* index into towers array, -1 = none */
    bool game_over;
    bool victory;
    bool debug_overlay;
    bool auto_waves;    /* automatically start next wave when current completes */
    bool wave_cleared;  /* tracks if bonus was already awarded for current wave */
    float fps;          /* updated from clock each frame */
    int game_speed;     /* 1 = normal, 2 = fast forward */
    Difficulty difficulty;
    AudioSystem *audio;    /* points to AppContext.audio, persists across games */
} GameState;

bool game_init(GameState *gs, const char *level_path, int screen_w, int screen_h, Difficulty diff);
void game_shutdown(GameState *gs);
void game_update(GameState *gs, Input *input, GameClock *clock);
void game_render(GameState *gs, UIContext *ui);

#endif /* TD_GAME_H */
