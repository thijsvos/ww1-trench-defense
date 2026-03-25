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
