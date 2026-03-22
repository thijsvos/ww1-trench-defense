#include "game.h"
#include "../core/log.h"
#include "../math/math_types.h"
#include "../math/math_utils.h"
#include "../math/vec.h"
#include "../ui/ui_widgets.h"
#include "../render/renderer.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

/* ------------------------------------------------------------------ */
/*  Constants                                                         */
/* ------------------------------------------------------------------ */

#define PAN_SPEED  5.0f
#define ZOOM_SPEED 0.1f

/* ------------------------------------------------------------------ */



/* Simple FNV-1a hash for generating stable UI widget IDs */
static uint32_t ui_id(const char *tag, int extra) {
    uint32_t hash = 2166136261u;
    while (*tag) {
        hash ^= (uint32_t)(unsigned char)*tag++;
        hash *= 16777619u;
    }
    hash ^= (uint32_t)extra * 2654435761u;
    return hash;
}

/* Projectile speed by damage type */
static const float PROJ_SPEED[] = {
    [DAMAGE_BULLET]          = 15.0f,
    [DAMAGE_PIERCING]        = 18.0f,
    [DAMAGE_EXPLOSIVE]       = 8.0f,
    [DAMAGE_HEAVY_EXPLOSIVE] = 6.0f,
    [DAMAGE_CHEMICAL]        = 5.0f,
    [DAMAGE_FIRE]            = 10.0f,
};


/* ------------------------------------------------------------------ */
/*  Init / Shutdown                                                   */
/* ------------------------------------------------------------------ */

/* Difficulty multiplier tables */
static const char *DIFF_NAMES[DIFF_COUNT] = {
    "Fresh Recruit", "Regular Tommy", "Trench Veteran", "Kaiserschlacht"
};
static const float DIFF_GOLD_MULT[DIFF_COUNT]     = { 1.5f, 1.0f, 0.75f, 0.5f };
static const float DIFF_LIVES_MULT[DIFF_COUNT]    = { 1.5f, 1.0f, 0.75f, 0.5f };
static const float DIFF_ENEMY_HP_MULT[DIFF_COUNT] = { 0.7f, 1.0f, 1.3f,  1.8f };
static const float DIFF_ENEMY_SPD_MULT[DIFF_COUNT]= { 0.85f,1.0f, 1.1f,  1.25f };
static const float DIFF_REWARD_MULT[DIFF_COUNT]   = { 1.3f, 1.0f, 0.8f,  0.6f };

bool game_init(GameState *gs, const char *level_path, int screen_w, int screen_h, Difficulty diff) {
    memset(gs, 0, sizeof(*gs));
    gs->difficulty = diff;

    /* Load level definition */
    if (!level_load(&gs->level_def, level_path)) {
        LOG_ERROR("Failed to load level: %s", level_path);
        return false;
    }

    /* Apply difficulty to starting resources */
    gs->level_def.starting_gold  = (int)(gs->level_def.starting_gold  * DIFF_GOLD_MULT[diff]);
    gs->level_def.starting_lives = (int)(gs->level_def.starting_lives * DIFF_LIVES_MULT[diff]);
    if (gs->level_def.starting_lives < 1) gs->level_def.starting_lives = 1;

    /* Apply level to map and paths */
    level_apply(&gs->level_def, &gs->map, &gs->paths);

    /* Init subsystems */
    enemy_manager_init(&gs->enemies);
    wave_manager_init(&gs->waves, gs->level_def.num_waves);
    wave_generate_waves(&gs->waves, gs->level_def.num_waves, gs->paths.path_count);
    gs->waves.diff_hp_mult     = DIFF_ENEMY_HP_MULT[diff];
    gs->waves.diff_spd_mult    = DIFF_ENEMY_SPD_MULT[diff];
    gs->waves.diff_reward_mult = DIFF_REWARD_MULT[diff];
    tower_manager_init(&gs->towers);
    projectile_manager_init(&gs->projectiles);
    economy_init(&gs->economy, gs->level_def.starting_gold, gs->level_def.starting_lives);

    LOG_INFO("Difficulty: %s (gold x%.0f%%, lives x%.0f%%, enemy HP x%.0f%%, speed x%.0f%%)",
             DIFF_NAMES[diff],
             DIFF_GOLD_MULT[diff] * 100, DIFF_LIVES_MULT[diff] * 100,
             DIFF_ENEMY_HP_MULT[diff] * 100, DIFF_ENEMY_SPD_MULT[diff] * 100);
    particle_system_init(&gs->particles);

    /* Camera */
    camera_init(&gs->camera, screen_w, screen_h);
    gs->camera.position = vec3((float)gs->map.width / 2.0f, 0.0f,
                               (float)gs->map.height / 2.0f);
    camera_update(&gs->camera);

    /* Sprite batch */
    if (!sprite_batch_init(&gs->batch)) {
        LOG_ERROR("Failed to init sprite batch");
        return false;
    }

    /* Generate procedural sprite atlas */
    if (!atlas_generate(&gs->atlas)) {
        LOG_ERROR("Failed to generate sprite atlas");
        return false;
    }

    /* Defaults */
    gs->selected_tower = TOWER_MACHINE_GUN;
    gs->hover_tile = ivec2(-1, -1);
    gs->selected_tower_index = -1;
    gs->game_over = false;
    gs->victory = false;
    gs->debug_overlay = false;
    gs->game_speed = 1;

    /* Debug draw */
    debug_draw_init(&gs->debug_draw, &gs->batch);



    /* Don't auto-start — let player set up before pressing SPACE */

    LOG_INFO("Game started - %s (Gold: %d, Lives: %d)",
             gs->level_def.name, gs->economy.gold, gs->economy.lives);
    return true;
}

void game_shutdown(GameState *gs) {
    atlas_destroy(&gs->atlas);
    sprite_batch_destroy(&gs->batch);
    LOG_INFO("Game state shut down");
}

/* ------------------------------------------------------------------ */
/*  Update                                                            */
/* ------------------------------------------------------------------ */

void game_update(GameState *gs, Input *input, GameClock *clock) {
    /* ---- One-shot input (once per frame, outside fixed timestep) ---- */
    if (!gs->game_over) {
        /* Tower selection (number keys 1-8) */
        for (int i = 0; i < TOWER_TYPE_COUNT; i++) {
            if (input_key_pressed(input, GLFW_KEY_1 + i)) {
                gs->selected_tower = (TowerType)i;
                gs->selected_tower_index = -1;
            }
        }

        /* Check if mouse is over UI regions (skip game clicks if so) */
        float mx = (float)input->mouse_x;
        float my = (float)input->mouse_y;
        float vw = (float)gs->camera.viewport_width;
        float vh = (float)gs->camera.viewport_height;
        bool mouse_over_ui = false;
        /* Top bar */
        if (my < 36.0f) mouse_over_ui = true;
        /* Bottom bar */
        if (my > vh - 64.0f) mouse_over_ui = true;
        /* Right panel (when a tower is selected) */
        if (gs->selected_tower_index >= 0 &&
            mx > vw - 190.0f && my > 46.0f && my < 300.0f)
            mouse_over_ui = true;

        /* Left-click: place tower or select existing tower */
        if (input_mouse_pressed(input, GLFW_MOUSE_BUTTON_LEFT) && !mouse_over_ui) {
            if (gs->hover_tile.x >= 0 && gs->hover_tile.y >= 0) {
                int clicked_tower = -1;
                for (int i = 0; i < gs->towers.count; i++) {
                    Tower *t = &gs->towers.towers[i];
                    if (t->tile.x == gs->hover_tile.x &&
                        t->tile.y == gs->hover_tile.y) {
                        clicked_tower = i;
                        break;
                    }
                }

                if (clicked_tower >= 0) {
                    gs->selected_tower_index = clicked_tower;
                } else if (gs->selected_tower >= 0 && gs->selected_tower < TOWER_TYPE_COUNT) {
                    gs->selected_tower_index = -1;
                    const TowerDef *def = tower_get_def(gs->selected_tower);
                    bool can_place = map_is_buildable(&gs->map,
                                                       gs->hover_tile.x,
                                                       gs->hover_tile.y);
                    if (gs->selected_tower == TOWER_BARBED_WIRE) {
                        Tile *t = map_get_tile(&gs->map,
                                                gs->hover_tile.x,
                                                gs->hover_tile.y);
                        can_place = (t && t->type == TILE_TRENCH && !t->occupied);
                    }
                    if (can_place && economy_can_afford(&gs->economy, def->cost)) {
                        Tower *tower = tower_place(&gs->towers, gs->selected_tower,
                                                    gs->hover_tile.x,
                                                    gs->hover_tile.y);
                        if (tower) {
                            economy_spend(&gs->economy, def->cost);
                            Tile *t = map_get_tile(&gs->map,
                                                    gs->hover_tile.x,
                                                    gs->hover_tile.y);
                            if (t) t->occupied = true;
                            /* Select the just-placed tower, deselect tower type */
                            gs->selected_tower = -1;
                            for (int ti = 0; ti < gs->towers.count; ti++) {
                                if (gs->towers.towers[ti].tile.x == gs->hover_tile.x &&
                                    gs->towers.towers[ti].tile.y == gs->hover_tile.y) {
                                    gs->selected_tower_index = ti;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        /* Right-click: sell selected tower */
        if (input_mouse_pressed(input, GLFW_MOUSE_BUTTON_RIGHT)) {
            if (gs->selected_tower_index >= 0 &&
                gs->selected_tower_index < gs->towers.count) {
                Tower *t = &gs->towers.towers[gs->selected_tower_index];
                int sell_gold = economy_sell_value(t->total_invested);
                economy_earn(&gs->economy, sell_gold);
                Tile *tile = map_get_tile(&gs->map, t->tile.x, t->tile.y);
                if (tile) tile->occupied = false;
                tower_remove(&gs->towers, gs->selected_tower_index);
                gs->selected_tower_index = -1;
                LOG_INFO("Tower sold for %d gold", sell_gold);
            }
        }

        /* SPACE: start next wave */
        if (input_key_pressed(input, GLFW_KEY_SPACE)) {
            bool ready = !gs->waves.wave_active &&
                         gs->enemies.count == 0 &&
                         !wave_all_complete(&gs->waves);
            if (ready) {
                wave_start_next(&gs->waves);
                gs->wave_cleared = false;
                LOG_INFO("Wave %d started", gs->waves.current_wave + 1);
            }
        }

        /* TAB: toggle auto-wave */
        if (input_key_pressed(input, GLFW_KEY_TAB)) {
            gs->auto_waves = !gs->auto_waves;
            LOG_INFO("Auto-waves %s", gs->auto_waves ? "ON" : "OFF");
        }

        /* F: cycle speed 1x → 2x → 5x → 1x */
        if (input_key_pressed(input, GLFW_KEY_F)) {
            if (gs->game_speed == 1) gs->game_speed = 2;
            else if (gs->game_speed == 2) gs->game_speed = 5;
            else gs->game_speed = 1;
            LOG_INFO("Game speed: %dx", gs->game_speed);
        }
    }

    /* ---- Fixed-timestep simulation ---- */
    /* At 2x speed, run two simulation ticks per normal tick */
    int ticks_per_step = gs->game_speed;
    while (clock_should_tick(clock)) {
      for (int speed_i = 0; speed_i < ticks_per_step; speed_i++) {
        float dt = (float)clock->fixed_dt;

        /* Camera panning (WASD) — continuous, so stays in fixed timestep */
        float pan = PAN_SPEED * dt;
        if (input_key_down(input, GLFW_KEY_W))
            camera_pan(&gs->camera, -pan, -pan);
        if (input_key_down(input, GLFW_KEY_S))
            camera_pan(&gs->camera, pan, pan);
        if (input_key_down(input, GLFW_KEY_A))
            camera_pan(&gs->camera, -pan, pan);
        if (input_key_down(input, GLFW_KEY_D))
            camera_pan(&gs->camera, pan, -pan);

        if (!gs->game_over) {
            /* Spawn enemies from waves */
            wave_update(&gs->waves, &gs->enemies, &gs->paths, dt);

            /* Update enemies */
            int leaked = enemy_update(&gs->enemies, &gs->paths, dt);
            if (leaked > 0) {
                economy_lose_lives(&gs->economy, leaked);
                if (economy_is_defeated(&gs->economy)) {
                    gs->game_over = true;
                    gs->victory = false;
                    LOG_INFO("DEFEAT - Trench line breached!");
                }
            }

            /* Update towers (targeting + cooldowns) */
            tower_update(&gs->towers, &gs->enemies, dt);

            /* Fire towers and spawn projectiles */
            for (int i = 0; i < gs->towers.count; i++) {
                Tower *tower = &gs->towers.towers[i];
                if (tower_try_fire(tower)) {
                    if (tower->target_index >= 0 &&
                        tower->target_index < gs->enemies.count) {
                        Enemy *target = &gs->enemies.enemies[tower->target_index];

                        if (tower->type == TOWER_FLAMETHROWER) {
                            /* Flamethrower: no projectile — direct cone AoE damage + flame stream */
                            particle_spawn_flame_stream(&gs->particles,
                                tower->position, target->position);

                            /* Damage all enemies in splash radius around target */
                            for (int j = 0; j < gs->enemies.count; j++) {
                                Enemy *e = &gs->enemies.enemies[j];
                                float d = vec2_distance(target->position, e->position);
                                if (d <= tower->splash_radius) {
                                    bool died = enemy_take_damage(e, tower->damage, tower->damage_type);
                                    enemy_apply_burn(e, tower->damage * 0.4f, 2.0f);
                                    if (died) {
                                        economy_earn(&gs->economy, e->reward);
                                        gs->economy.total_kills++;
                                    }
                                }
                            }
                        } else if (tower->type == TOWER_GAS) {
                            /* Gas dispenser: no projectile — spreads gas cloud at target area */
                            particle_spawn_gas_cloud(&gs->particles,
                                target->position, tower->splash_radius);

                            /* Damage + apply gas DoT to all enemies in splash radius */
                            for (int j = 0; j < gs->enemies.count; j++) {
                                Enemy *e = &gs->enemies.enemies[j];
                                float d = vec2_distance(target->position, e->position);
                                if (d <= tower->splash_radius) {
                                    bool died = enemy_take_damage(e, tower->damage, tower->damage_type);
                                    enemy_apply_gas(e, tower->damage * 0.5f, 3.0f);
                                    if (died) {
                                        economy_earn(&gs->economy, e->reward);
                                        gs->economy.total_kills++;
                                    }
                                }
                            }
                        } else {
                            /* Normal tower: spawn projectile */
                            projectile_spawn(&gs->projectiles,
                                tower->position,
                                tower->target_index,
                                target->position,
                                PROJ_SPEED[tower->damage_type],
                                tower->damage,
                                tower->damage_type,
                                tower->splash_radius);
                            /* Muzzle flash */
                            particle_spawn_muzzle_flash(&gs->particles, tower->position);
                        }
                    }
                }
            }

            /* Record arcing shell positions that are about to impact */
            Vec2 arc_impacts[32];
            int arc_impact_count = 0;
            for (int pi = 0; pi < gs->projectiles.count && arc_impact_count < 32; pi++) {
                Projectile *pr = &gs->projectiles.projectiles[pi];
                if (pr->arcing && pr->arc_progress + dt / pr->arc_duration >= 1.0f) {
                    arc_impacts[arc_impact_count++] = pr->arc_target;
                }
            }

            /* Update projectiles — track kills for particle effects */
            int prev_count = gs->enemies.count;
            int gold_earned = 0;
            int kills = projectile_update(&gs->projectiles, &gs->enemies,
                                           dt, &gold_earned);

            /* Spawn explosions at recorded impact points */
            for (int ai = 0; ai < arc_impact_count; ai++) {
                particle_spawn_explosion(&gs->particles, arc_impacts[ai], 0.6f);
                particle_spawn_smoke(&gs->particles, arc_impacts[ai], 6);
            }
            if (gold_earned > 0)
                economy_earn(&gs->economy, gold_earned);
            gs->economy.total_kills += kills;

            /* Spawn particles for killed enemies */
            if (kills > 0) {
                /* Check which enemies were removed (they're swapped to end) */
                for (int i = gs->enemies.count; i < prev_count && i < MAX_ENEMIES; i++) {
                    Enemy *dead = &gs->enemies.enemies[i];
                    particle_spawn_explosion(&gs->particles, dead->position, 0.3f);
                    particle_spawn_blood(&gs->particles, dead->position);
                }
            }

            /* Update particles */
            particle_update(&gs->particles, dt);

            /* Check wave complete (spawning done + all enemies dead) */
            if (wave_is_complete(&gs->waves, &gs->enemies) &&
                gs->waves.current_wave >= 0 && !gs->wave_cleared) {
                gs->wave_cleared = true;
                WaveDef *wd = &gs->waves.waves[gs->waves.current_wave];
                economy_earn(&gs->economy, wd->bonus_gold);
                LOG_INFO("Wave %d complete! +%d gold (Total: %d)",
                         gs->waves.current_wave + 1, wd->bonus_gold,
                         gs->economy.gold);

                if (wave_all_complete(&gs->waves)) {
                    gs->game_over = true;
                    gs->victory = true;
                    LOG_INFO("VICTORY! Score: %d", gs->economy.score);
                } else if (gs->auto_waves) {
                    wave_start_next(&gs->waves);
                    gs->wave_cleared = false;
                    LOG_INFO("Wave %d auto-started", gs->waves.current_wave + 1);
                }
            }
        }
      } /* end speed_i loop */
    }

    /* ---- Per-frame updates (outside fixed timestep) ---- */
    gs->fps = clock->fps;

    /* Scroll zoom */
    if (input->scroll_y != 0.0) {
        camera_set_zoom(&gs->camera,
                        gs->camera.zoom + (float)input->scroll_y * ZOOM_SPEED);
    }

    /* Update camera */
    camera_update(&gs->camera);

    /* Mouse -> world -> tile */
    Vec3 world_pos = camera_screen_to_world(&gs->camera,
        (float)input->mouse_x, (float)input->mouse_y);
    int tx = (int)world_pos.x;
    int tz = (int)world_pos.z;
    if (tx >= 0 && tx < gs->map.width && tz >= 0 && tz < gs->map.height)
        gs->hover_tile = ivec2(tx, tz);
    else
        gs->hover_tile = ivec2(-1, -1);
}

/* ------------------------------------------------------------------ */
/*  Render                                                            */
/* ------------------------------------------------------------------ */

void game_render(GameState *gs, UIContext *ui) {
    Vec4 white = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    /* ---- World rendering via sprite batch ---- */
    sprite_batch_begin(&gs->batch, &gs->camera.view_projection);

    /* 1. Map tiles (textured, with auto-tiled trenches) */
    for (int y = 0; y < gs->map.height; y++) {
        for (int x = 0; x < gs->map.width; x++) {
            Tile *tile = map_get_tile(&gs->map, x, y);
            Vec4 tint = white;
            if ((x + y) % 2 == 0) {
                tint = vec4(0.92f, 0.92f, 0.92f, 1.0f);
            }
            if (x == gs->hover_tile.x && y == gs->hover_tile.y) {
                tint.x += 0.15f;
                tint.y += 0.15f;
                tint.z += 0.15f;
            }

            TextureRegion *region;
            if (tile->type == TILE_TRENCH) {
                int mask = atlas_trench_mask(&gs->map, x, y);
                region = &gs->atlas.trench_variants[mask];
            } else {
                region = &gs->atlas.tiles[tile->type];
            }

            sprite_batch_draw_textured(&gs->batch,
                vec3((float)x, 0.0f, (float)y),
                vec2(1.0f, 1.0f), region, tint);
        }
    }

    /* Range circle indicator (simple white outline on the ground) */
    {
        float range = 0.0f;
        float cx = 0.0f, cz = 0.0f;
        bool show_range = false;

        /* Selected placed tower — show its range */
        if (gs->selected_tower_index >= 0 &&
            gs->selected_tower_index < gs->towers.count) {
            Tower *t = &gs->towers.towers[gs->selected_tower_index];
            range = t->range;
            cx = t->position.x;
            cz = t->position.y;
            show_range = true;
        }
        /* Hovering a valid placement tile — show preview range */
        else if (gs->hover_tile.x >= 0 && gs->hover_tile.y >= 0 &&
                 gs->selected_tower >= 0 && gs->selected_tower < TOWER_TYPE_COUNT) {
            bool can_place = map_is_buildable(&gs->map, gs->hover_tile.x, gs->hover_tile.y);
            if (gs->selected_tower == TOWER_BARBED_WIRE) {
                Tile *ht = map_get_tile(&gs->map, gs->hover_tile.x, gs->hover_tile.y);
                can_place = (ht && ht->type == TILE_TRENCH && !ht->occupied);
            }
            if (can_place) {
                const TowerDef *def = tower_get_def(gs->selected_tower);
                if (def->range > 0.5f) {
                    range = def->range;
                    cx = (float)gs->hover_tile.x + 0.5f;
                    cz = (float)gs->hover_tile.y + 0.5f;
                    show_range = true;
                }
            }
        }

        if (show_range) {
            /* Draw circle as connected line segments using thin quads */
            int seg = 48;
            float step = 2.0f * 3.14159265f / (float)seg;
            float thickness = 0.03f;
            Vec4 ring_col = vec4(1.0f, 1.0f, 1.0f, 0.5f);
            for (int s = 0; s < seg; s++) {
                /* Skip every other segment for dashed line */
                if (s % 2 != 0) continue;
                float a0 = (float)s * step;
                float a1 = (float)(s + 1) * step;
                float x0 = cx + cosf(a0) * range;
                float z0 = cz + sinf(a0) * range;
                float x1 = cx + cosf(a1) * range;
                float z1 = cz + sinf(a1) * range;
                /* Direction along segment */
                float dx = x1 - x0;
                float dz = z1 - z0;
                float len = sqrtf(dx * dx + dz * dz);
                if (len < 0.001f) continue;
                /* Perpendicular for thickness */
                float nx = -dz / len * thickness * 0.5f;
                float nz =  dx / len * thickness * 0.5f;
                /* Quad as a thin line segment */
                int base = gs->batch.quad_count * 4;
                if (gs->batch.quad_count >= SPRITE_BATCH_MAX_QUADS) {
                    sprite_batch_end(&gs->batch);
                    sprite_batch_begin(&gs->batch, &gs->camera.view_projection);
                }
                /* Match atlas white texture */
                uint32_t tex_id = gs->atlas.white.texture->id;
                if (gs->batch.current_texture != 0 && gs->batch.current_texture != tex_id) {
                    sprite_batch_end(&gs->batch);
                    sprite_batch_begin(&gs->batch, &gs->camera.view_projection);
                }
                gs->batch.current_texture = tex_id;
                base = gs->batch.quad_count * 4;
                gs->batch.vertices[base + 0].position = vec3(x0 - nx, 0.005f, z0 - nz);
                gs->batch.vertices[base + 0].uv = vec2(gs->atlas.white.u0, gs->atlas.white.v0);
                gs->batch.vertices[base + 0].color = ring_col;
                gs->batch.vertices[base + 1].position = vec3(x0 + nx, 0.005f, z0 + nz);
                gs->batch.vertices[base + 1].uv = vec2(gs->atlas.white.u1, gs->atlas.white.v0);
                gs->batch.vertices[base + 1].color = ring_col;
                gs->batch.vertices[base + 2].position = vec3(x1 + nx, 0.005f, z1 + nz);
                gs->batch.vertices[base + 2].uv = vec2(gs->atlas.white.u1, gs->atlas.white.v1);
                gs->batch.vertices[base + 2].color = ring_col;
                gs->batch.vertices[base + 3].position = vec3(x1 - nx, 0.005f, z1 - nz);
                gs->batch.vertices[base + 3].uv = vec2(gs->atlas.white.u0, gs->atlas.white.v1);
                gs->batch.vertices[base + 3].color = ring_col;
                gs->batch.quad_count++;
            }
        }
    }

    /* Decorative no man's land scene in center of level 5 */
    if (gs->map.width == 24 && gs->map.height == 20) {
        float cx = (float)gs->map.width * 0.5f;
        float cz = (float)gs->map.height * 0.5f;
        sprite_batch_draw_textured(&gs->batch,
            vec3(cx - 3.5f, 0.009f, cz - 3.5f),
            vec2(7.0f, 7.0f),
            &gs->atlas.fort, white);
    }

    /* Camera billboard vectors */
    Vec3 cam_right = gs->camera.right;

    /* 2. Towers (billboard — stand upright, face camera) */
    for (int i = 0; i < gs->towers.count; i++) {
        Tower *t = &gs->towers.towers[i];
        Vec4 tint = white;

        if (i == gs->selected_tower_index)
            tint = vec4(1.3f, 1.3f, 1.3f, 1.0f);
        if (t->upgrade_level == 1)
            tint.z *= 1.15f;
        else if (t->upgrade_level == 2)
            tint.x *= 1.15f;

        /* Barbed wire stays flat on the ground, rotated to match trench direction */
        if (t->type == TOWER_BARBED_WIRE) {
            float sz = 0.9f;
            /* Check trench neighbors to determine track direction */
            int mask = atlas_trench_mask(&gs->map, t->tile.x, t->tile.y);
            bool has_ns = (mask & TRENCH_N) || (mask & TRENCH_S);
            bool has_ew = (mask & TRENCH_E) || (mask & TRENCH_W);
            float angle = 0.0f;
            if (has_ns && !has_ew)
                angle = 3.14159265f * 0.5f; /* vertical track: rotate 90° */
            /* For corners/junctions, default horizontal is fine */
            sprite_batch_draw_rotated(&gs->batch,
                vec3(t->position.x, 0.01f, t->position.y),
                vec2(sz, sz), angle,
                &gs->atlas.towers[t->type], tint);
        } else {
            /* Standing billboard: width on ground, height going up */
            float w = 0.8f;
            float h = (t->type == TOWER_OBSERVATION) ? 1.2f :
                      (t->type == TOWER_ARTILLERY)   ? 0.7f :
                      (t->type == TOWER_SNIPER)      ? 1.0f : 0.8f;
            /* Observation balloon: bobs and sways gently */
            float y_off = 0.0f;
            float sway_x = 0.0f, sway_z = 0.0f;
            if (t->type == TOWER_OBSERVATION) {
                float time = (float)glfwGetTime();
                float phase = (float)(i * 2);
                y_off = sinf(time * 1.5f + phase) * 0.08f;
                sway_x = sinf(time * 0.4f + phase) * 0.10f;
                sway_z = cosf(time * 0.4f + phase) * 0.10f;
            }
            sprite_batch_draw_billboard(&gs->batch,
                vec3(t->position.x + sway_x, y_off, t->position.y + sway_z),
                vec2(w, h), cam_right,
                &gs->atlas.towers[t->type], tint);
        }
    }

    /* 3. Enemies (billboard — stand upright, face camera) */
    for (int i = 0; i < gs->enemies.count; i++) {
        Enemy *e = &gs->enemies.enemies[i];
        if (e->burrowed) continue;

        Vec4 tint = white;
        if (e->slow_timer > 0.0f)
            tint = vec4(0.7f, 0.7f, 1.0f, 1.0f);
        if (e->burn_timer > 0.0f)
            tint = vec4(1.3f, 0.8f, 0.5f, 1.0f);
        if (e->gas_timer > 0.0f)
            tint = vec4(0.9f, 1.2f, 0.5f, 1.0f);

        /* Vehicles are wider and shorter, soldiers are taller */
        float w, h;
        if (e->type == ENEMY_TANK) {
            w = 0.7f; h = 0.45f;
        } else if (e->type == ENEMY_ARMORED_CAR) {
            w = 0.5f; h = 0.4f;
        } else if (e->type == ENEMY_CAVALRY) {
            w = 0.45f; h = 0.5f;
        } else {
            w = 0.35f; h = 0.5f; /* infantry-sized */
        }

        /* Animated walk frame */
        int anim_frame = ((int)(e->anim_timer * 6.0f)) % ENEMY_ANIM_FRAMES;
        sprite_batch_draw_billboard(&gs->batch,
            vec3(e->position.x, 0.0f, e->position.y),
            vec2(w, h), cam_right,
            &gs->atlas.enemy_anim[e->type][anim_frame], tint);

        /* === Role indicators and aura effects for special enemies === */

        /* Medic: healing aura (pulsing green circle on ground) */
        if (e->type == ENEMY_MEDIC) {
            float pulse = 0.5f + 0.3f * sinf((float)glfwGetTime() * 4.0f);
            float aura_r = 2.0f; /* matches MEDIC_HEAL_RANGE */
            int seg = 20;
            float step = 2.0f * 3.14159265f / (float)seg;
            for (int s = 0; s < seg; s += 2) {
                float a = (float)s * step;
                float ax = e->position.x + cosf(a) * aura_r;
                float az = e->position.y + sinf(a) * aura_r;
                sprite_batch_draw_textured(&gs->batch,
                    vec3(ax - 0.04f, 0.006f, az - 0.04f),
                    vec2(0.08f, 0.08f), &gs->atlas.white,
                    vec4(0.2f, 0.9f, 0.2f, 0.25f * pulse));
            }
        }

        /* Officer: speed aura (pulsing yellow circle on ground) */
        if (e->type == ENEMY_OFFICER) {
            float pulse = 0.5f + 0.3f * sinf((float)glfwGetTime() * 3.0f);
            float aura_r = 2.5f; /* matches OFFICER_BUFF_RANGE */
            int seg = 20;
            float step = 2.0f * 3.14159265f / (float)seg;
            for (int s = 0; s < seg; s += 2) {
                float a = (float)s * step;
                float ax = e->position.x + cosf(a) * aura_r;
                float az = e->position.y + sinf(a) * aura_r;
                sprite_batch_draw_textured(&gs->batch,
                    vec3(ax - 0.04f, 0.006f, az - 0.04f),
                    vec2(0.08f, 0.08f), &gs->atlas.white,
                    vec4(0.9f, 0.8f, 0.1f, 0.2f * pulse));
            }
        }

        /* Health bar + role icon (depth test off so always visible) */
        {
            float hp_frac = e->hp / e->max_hp;
            if (hp_frac < 0.0f) hp_frac = 0.0f;

            bool has_overlay = (hp_frac < 1.0f && hp_frac > 0.0f) ||
                               e->type == ENEMY_MEDIC || e->type == ENEMY_OFFICER ||
                               e->type == ENEMY_TANK || e->type == ENEMY_TUNNEL_SAPPER;

            if (has_overlay) {
                glDisable(GL_DEPTH_TEST);

                /* Health bar */
                if (hp_frac < 1.0f && hp_frac > 0.0f) {
                    float bar_w = w + 0.1f;
                    float bar_h_px = 0.04f;
                    float bar_y = h + 0.06f;
                    sprite_batch_draw_billboard(&gs->batch,
                        vec3(e->position.x, bar_y, e->position.y),
                        vec2(bar_w * hp_frac, bar_h_px), cam_right,
                        &gs->atlas.white,
                        vec4(1.0f - hp_frac, hp_frac, 0.0f, 1.0f));
                }

                /* Role icon above enemy */
                float icon_y = h + 0.14f;
                float icon_s = 0.12f;
                const char *icon = NULL;
                Vec4 icon_col = white;

                switch (e->type) {
                    case ENEMY_MEDIC:
                        icon = "+";
                        icon_col = vec4(1.0f, 0.3f, 0.3f, 1.0f); /* red cross */
                        break;
                    case ENEMY_OFFICER:
                        icon = "*";
                        icon_col = vec4(1.0f, 0.85f, 0.2f, 1.0f); /* gold star */
                        break;
                    case ENEMY_TANK:
                        icon = "#";
                        icon_col = vec4(0.7f, 0.7f, 0.75f, 0.8f); /* steel */
                        break;
                    case ENEMY_TUNNEL_SAPPER:
                        icon = "?";
                        icon_col = vec4(0.6f, 0.45f, 0.3f, 0.8f); /* earth */
                        break;
                    default: break;
                }

                if (icon) {
                    /* Draw icon as a small label centered above the enemy */
                    float lbl_scale = 1.5f;
                    float lbl_w = 6.0f * lbl_scale;
                    /* Convert world position to approximate screen position
                       — use billboard center offset by cam_right */
                    sprite_batch_draw_billboard(&gs->batch,
                        vec3(e->position.x, icon_y, e->position.y),
                        vec2(icon_s, icon_s), cam_right,
                        &gs->atlas.white, icon_col);
                    (void)lbl_w; (void)lbl_scale;
                }

                sprite_batch_end(&gs->batch);
                glEnable(GL_DEPTH_TEST);
                sprite_batch_begin(&gs->batch, &gs->camera.view_projection);
            }
        }
    }

    /* 4. Projectiles */
    for (int i = 0; i < gs->projectiles.count; i++) {
        Projectile *p = &gs->projectiles.projectiles[i];

        if (p->arcing) {
            /* Arcing mortar/artillery shell — render as billboard at arc height */
            float psize = (p->damage_type == DAMAGE_HEAVY_EXPLOSIVE) ? 0.35f : 0.28f;
            sprite_batch_draw_billboard(&gs->batch,
                vec3(p->position.x, p->height, p->position.y),
                vec2(psize, psize), cam_right,
                &gs->atlas.projectiles[p->damage_type],
                vec4(1.2f, 1.2f, 1.0f, 1.0f)); /* slightly brighter tint */

            /* Small smoke trail behind the shell */
            float trail_size = psize * 0.6f;
            sprite_batch_draw_billboard(&gs->batch,
                vec3(p->position.x, p->height + 0.05f, p->position.y),
                vec2(trail_size, trail_size), cam_right,
                &gs->atlas.white,
                vec4(0.6f, 0.6f, 0.5f, 0.4f));

            /* Shadow on the ground beneath the shell */
            float shadow_size = 0.1f + p->height * 0.05f;
            sprite_batch_draw_textured(&gs->batch,
                vec3(p->position.x - shadow_size, 0.003f, p->position.y - shadow_size),
                vec2(shadow_size * 2, shadow_size * 2),
                &gs->atlas.white,
                vec4(0.0f, 0.0f, 0.0f, 0.25f));
        } else {
            /* Straight-line projectile — flat on ground as before */
            float psize = 0.18f;
            sprite_batch_draw_textured(&gs->batch,
                vec3(p->position.x - psize / 2, 0.04f,
                     p->position.y - psize / 2),
                vec2(psize, psize),
                &gs->atlas.projectiles[p->damage_type], white);
        }
    }

    /* 5. Particles */
    particle_render(&gs->particles, &gs->batch, &gs->atlas.soft_circle);

    /* 6. Debug overlay */
    if (gs->debug_overlay) {
        debug_draw_grid(&gs->debug_draw, &gs->camera, gs->map.width, gs->map.height);
        debug_draw_paths(&gs->debug_draw, &gs->camera, &gs->paths);
        /* Show range of selected tower */
        if (gs->selected_tower_index >= 0 &&
            gs->selected_tower_index < gs->towers.count) {
            Tower *t = &gs->towers.towers[gs->selected_tower_index];
            debug_draw_tower_range(&gs->debug_draw, &gs->camera,
                                    t->position, t->range);
        }
        /* Show hover tile marker */
        if (gs->hover_tile.x >= 0) {
            debug_draw_marker(&gs->debug_draw, &gs->camera,
                vec2((float)gs->hover_tile.x + 0.5f, (float)gs->hover_tile.y + 0.5f),
                vec4(1.0f, 1.0f, 1.0f, 0.5f));
        }
    }

    sprite_batch_end(&gs->batch);

    /* ---- HUD via UI system ---- */
    float sw = (float)ui->screen_w;
    float sh = (float)ui->screen_h;
    Vec4 bar_bg = vec4(0.12f, 0.10f, 0.08f, 0.85f);

    /* Top bar background */
    ui_panel(ui, 0, 0, sw, 36, bar_bg);

    /* Top bar info: Gold | Lives | Wave | Kills */
    {
        char buf[64];
        float x = 10.0f;

        snprintf(buf, sizeof(buf), "Gold: %d", gs->economy.gold);
        ui_label(ui, x, 10.0f, buf, vec4(1.0f, 0.85f, 0.30f, 1.0f), 2.0f);
        x += 130.0f;

        snprintf(buf, sizeof(buf), "Lives: %d", gs->economy.lives);
        ui_label(ui, x, 10.0f, buf, vec4(0.90f, 0.30f, 0.30f, 1.0f), 2.0f);
        x += 130.0f;

        snprintf(buf, sizeof(buf), "Wave: %d/%d",
                 gs->waves.current_wave + 1, gs->waves.wave_count);
        ui_label(ui, x, 10.0f, buf, vec4(0.70f, 0.80f, 1.00f, 1.0f), 2.0f);
        x += 150.0f;

        snprintf(buf, sizeof(buf), "Kills: %d", gs->economy.total_kills);
        ui_label(ui, x, 10.0f, buf, vec4(0.90f, 0.90f, 0.90f, 1.0f), 2.0f);
        x += 120.0f;

        /* Auto-wave indicator */
        const char *auto_label = gs->auto_waves ? "[TAB] Auto: ON" : "[TAB] Auto: OFF";
        Vec4 auto_col = gs->auto_waves
            ? vec4(0.40f, 0.90f, 0.40f, 1.0f)
            : vec4(0.50f, 0.50f, 0.50f, 1.0f);
        ui_label(ui, x, 10.0f, auto_label, auto_col, 2.0f);
        x += (float)strlen(auto_label) * 6.0f * 2.0f + 16.0f;

        /* Speed indicator */
        const char *speed_label = gs->game_speed == 5 ? "[F] 5x" :
                                  gs->game_speed == 2 ? "[F] 2x" : "[F] 1x";
        Vec4 speed_col = gs->game_speed == 5
            ? vec4(1.0f, 0.4f, 0.2f, 1.0f)
            : gs->game_speed == 2
            ? vec4(1.0f, 0.85f, 0.30f, 1.0f)
            : vec4(0.50f, 0.50f, 0.50f, 1.0f);
        ui_label(ui, x, 10.0f, speed_label, speed_col, 2.0f);

        /* Difficulty + FPS — top right */
        snprintf(buf, sizeof(buf), "%s | %.0f FPS",
                 DIFF_NAMES[gs->difficulty], gs->fps);
        float fps_scale = 2.0f;
        float fps_w = (float)strlen(buf) * 6.0f * fps_scale;
        Vec4 fps_col = gs->fps >= 55.0f ? vec4(0.40f, 0.90f, 0.40f, 1.0f)
                                        : vec4(0.90f, 0.30f, 0.30f, 1.0f);
        ui_label(ui, sw - fps_w - 10.0f, 10.0f, buf, fps_col, fps_scale);
    }

    /* Bottom bar: tower selection buttons with names and tooltips */
    {
        static const char *tower_short[] = {
            "MG", "Mort", "Snpr", "Wire", "Art", "Gas", "Flm", "Obs"
        };
        float bar_h = 72.0f;
        float bar_y = sh - bar_h;
        ui_panel(ui, 0, bar_y, sw, bar_h, bar_bg);

        float btn_w = 52.0f;
        float btn_h = 44.0f;
        float spacing = 6.0f;
        float total_w = (float)TOWER_TYPE_COUNT * btn_w +
                        (float)(TOWER_TYPE_COUNT - 1) * spacing;
        float start_x = (sw - total_w) / 2.0f;

        int hovered_tower = -1;

        for (int i = 0; i < TOWER_TYPE_COUNT; i++) {
            float bx = start_x + (float)i * (btn_w + spacing);
            float by = bar_y + 3.0f;
            const TowerDef *def = tower_get_def((TowerType)i);
            bool selected = (gs->selected_tower == (TowerType)i);
            bool afford = economy_can_afford(&gs->economy, def->cost);

            /* Button background */
            Vec4 btn_bg = vec4(0.18f, 0.16f, 0.14f, 1.0f);
            bool hovered = (ui->mouse_x >= bx && ui->mouse_x < bx + btn_w &&
                            ui->mouse_y >= by && ui->mouse_y < by + btn_h);
            if (hovered) {
                btn_bg = vec4(0.28f, 0.26f, 0.22f, 1.0f);
                hovered_tower = i;
            }
            if (selected) {
                /* Selection border */
                ui_draw_rect(ui, bx - 2, by - 2, btn_w + 4, btn_h + 4,
                             vec4(1.0f, 0.85f, 0.3f, 1.0f));
            }
            ui_draw_rect(ui, bx, by, btn_w, btn_h, btn_bg);

            /* Tower name inside button */
            Vec4 name_col = afford ? vec4(0.90f, 0.90f, 0.90f, 1.0f)
                                   : vec4(0.50f, 0.40f, 0.40f, 1.0f);
            float name_scale = 1.5f;
            float tw = (float)strlen(tower_short[i]) * 6.0f * name_scale;
            ui_label(ui, bx + (btn_w - tw) / 2.0f, by + 6.0f,
                     tower_short[i], name_col, name_scale);

            /* Cost label below name */
            char cost_str[16];
            snprintf(cost_str, sizeof(cost_str), "$%d", def->cost);
            Vec4 cost_col = afford ? vec4(1.0f, 0.85f, 0.30f, 1.0f)
                                   : vec4(0.60f, 0.25f, 0.25f, 1.0f);
            float cw = (float)strlen(cost_str) * 6.0f * 1.5f;
            ui_label(ui, bx + (btn_w - cw) / 2.0f, by + 24.0f,
                     cost_str, cost_col, 1.5f);

            /* Click handler */
            if (ui->mouse_pressed && hovered) {
                gs->selected_tower = (TowerType)i;
                gs->selected_tower_index = -1;
            }

            /* Hotkey hint */
            char key[4];
            snprintf(key, sizeof(key), "%d", i + 1);
            ui_label(ui, bx + 2.0f, by + btn_h - 10.0f, key,
                     vec4(0.40f, 0.40f, 0.40f, 1.0f), 1.0f);
        }

        /* Tooltip for hovered tower */
        if (hovered_tower >= 0) {
            const TowerDef *def = tower_get_def((TowerType)hovered_tower);
            static const char *dmg_names[] = {
                "Bullet", "Piercing", "Explosive", "Hvy Expl", "Chemical", "Fire"
            };
            char tip[128];
            snprintf(tip, sizeof(tip), "%s | Dmg:%.0f %s | Rng:%.0f | Rate:%.1f/s",
                     def->name, def->damage, dmg_names[def->damage_type],
                     def->range, def->fire_rate);
            float tip_scale = 1.5f;
            float tip_w = (float)strlen(tip) * 6.0f * tip_scale + 12.0f;
            float tip_h = 7.0f * tip_scale + 10.0f;
            float tip_x = (float)ui->mouse_x;
            float tip_y = bar_y - tip_h - 4.0f;
            if (tip_x + tip_w > sw) tip_x = sw - tip_w;
            ui_panel(ui, tip_x, tip_y, tip_w, tip_h,
                     vec4(0.10f, 0.08f, 0.06f, 0.95f));
            ui_label(ui, tip_x + 6.0f, tip_y + 5.0f, tip,
                     vec4(0.90f, 0.90f, 0.85f, 1.0f), tip_scale);
        }
    }

    /* Right panel: selected tower info */
    if (gs->selected_tower_index >= 0 &&
        gs->selected_tower_index < gs->towers.count) {
        Tower *t = &gs->towers.towers[gs->selected_tower_index];
        const TowerDef *def = tower_get_def(t->type);

        float pw = 180.0f;
        float ph = 250.0f;
        float px = sw - pw - 10.0f;
        float py = 46.0f;
        Vec4 panel_bg = vec4(0.12f, 0.10f, 0.08f, 0.90f);
        ui_panel(ui, px, py, pw, ph, panel_bg);

        float yy = py + 8.0f;
        Vec4 gray = vec4(0.70f, 0.70f, 0.70f, 1.0f);

        ui_label(ui, px + 8.0f, yy, def->name, white, 2.0f);
        yy += 24.0f;

        char buf[48];
        snprintf(buf, sizeof(buf), "Lvl: %d", t->upgrade_level + 1);
        ui_label(ui, px + 8.0f, yy, buf, gray, 1.5f);
        yy += 20.0f;

        snprintf(buf, sizeof(buf), "Dmg: %.0f", t->damage);
        ui_label(ui, px + 8.0f, yy, buf, gray, 1.5f);
        yy += 20.0f;

        snprintf(buf, sizeof(buf), "Range: %.1f", t->range);
        ui_label(ui, px + 8.0f, yy, buf, gray, 1.5f);
        yy += 20.0f;

        snprintf(buf, sizeof(buf), "Rate: %.1f/s", t->fire_rate);
        ui_label(ui, px + 8.0f, yy, buf, gray, 1.5f);
        yy += 22.0f;

        /* Target priority button (not for wire/observation) */
        if (t->type != TOWER_BARBED_WIRE && t->type != TOWER_OBSERVATION) {
            static const char *priority_names[] = {
                "First", "Last", "Strong", "Weak", "Near"
            };
            char pri_label[32];
            snprintf(pri_label, sizeof(pri_label), "Target: %s",
                     priority_names[t->priority]);
            if (ui_button(ui, ui_id("priority", gs->selected_tower_index),
                          px + 8.0f, yy, pw - 16.0f, 24.0f,
                          pri_label,
                          vec4(0.22f, 0.25f, 0.35f, 1.0f), white)) {
                t->priority = (t->priority + 1) % 5;
            }
            yy += 30.0f;
        }

        /* Upgrade button */
        if (t->upgrade_level < 2) {
            int up_cost = def->upgrade_cost[t->upgrade_level + 1];
            char up_label[32];
            snprintf(up_label, sizeof(up_label), "Upgrade $%d", up_cost);
            Vec4 up_bg = economy_can_afford(&gs->economy, up_cost)
                ? vec4(0.25f, 0.40f, 0.25f, 1.0f)
                : vec4(0.30f, 0.20f, 0.20f, 1.0f);
            if (ui_button(ui, ui_id("upgrade", gs->selected_tower_index),
                          px + 8.0f, yy, pw - 16.0f, 28.0f,
                          up_label, up_bg, white)) {
                if (economy_can_afford(&gs->economy, up_cost)) {
                    economy_spend(&gs->economy, up_cost);
                    tower_upgrade(t);
                    LOG_INFO("Tower upgraded to level %d", t->upgrade_level + 1);
                }
            }
            yy += 34.0f;
        }

        /* Sell button */
        int sell_val = economy_sell_value(t->total_invested);
        char sell_label[32];
        snprintf(sell_label, sizeof(sell_label), "Sell +$%d", sell_val);
        Vec4 sell_bg = vec4(0.50f, 0.20f, 0.20f, 1.0f);
        if (ui_button(ui, ui_id("sell", gs->selected_tower_index),
                      px + 8.0f, yy, pw - 16.0f, 28.0f,
                      sell_label, sell_bg, white)) {
            economy_earn(&gs->economy, sell_val);
            Tile *tile = map_get_tile(&gs->map, t->tile.x, t->tile.y);
            if (tile) tile->occupied = false;
            tower_remove(&gs->towers, gs->selected_tower_index);
            gs->selected_tower_index = -1;
        }
    }

    /* "Press SPACE" prompt when between waves */
    if (!gs->game_over && !gs->waves.wave_active &&
        gs->enemies.count == 0 && !wave_all_complete(&gs->waves)) {
        const char *prompt = gs->waves.current_wave < 0
            ? "Press [SPACE] to start"
            : "Press [SPACE] for next wave";
        float scale = 2.5f;
        float pw = (float)strlen(prompt) * 6.0f * scale;
        float px = (sw - pw) / 2.0f;
        float py = sh * 0.4f;
        /* Pulsing alpha for attention */
        float pulse = 0.55f + 0.35f * sinf((float)glfwGetTime() * 3.0f);
        Vec4 prompt_col = vec4(1.0f, 0.9f, 0.5f, pulse);
        ui_label(ui, px, py, prompt, prompt_col, scale);
    }

    /* Game over overlay */
    if (gs->game_over) {
        ui_draw_rect(ui, 0, 0, sw, sh,
                     vec4(0.0f, 0.0f, 0.0f, 0.60f));

        float box_w = 320.0f;
        float box_h = 100.0f;
        float bx = (sw - box_w) / 2.0f;
        float by = (sh - box_h) / 2.0f;
        ui_panel(ui, bx, by, box_w, box_h,
                 vec4(0.15f, 0.12f, 0.10f, 0.95f));

        const char *result_text = gs->victory ? "VICTORY!" : "DEFEAT!";
        Vec4 result_col = gs->victory
            ? vec4(0.30f, 0.90f, 0.30f, 1.0f)
            : vec4(0.90f, 0.20f, 0.20f, 1.0f);
        /* Approximate centering: 5px char width * scale 3 = 15px per char */
        float text_w = (float)strlen(result_text) * 15.0f;
        ui_label(ui, bx + (box_w - text_w) / 2.0f, by + 20.0f,
                 result_text, result_col, 3.0f);

        char stats[64];
        snprintf(stats, sizeof(stats), "Waves: %d  Kills: %d",
                 gs->waves.current_wave + 1, gs->economy.total_kills);
        float stats_w = (float)strlen(stats) * 10.0f;
        ui_label(ui, bx + (box_w - stats_w) / 2.0f, by + 60.0f,
                 stats, vec4(0.80f, 0.80f, 0.80f, 1.0f), 2.0f);
    }
}
