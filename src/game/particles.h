#ifndef TD_PARTICLES_H
#define TD_PARTICLES_H

#include <stdbool.h>
#include "../math/math_types.h"
#include "../render/sprite_batch.h"

#define MAX_PARTICLES 1024

typedef enum ParticleType {
    PARTICLE_SMOKE = 0,
    PARTICLE_EXPLOSION,
    PARTICLE_SPARK,
    PARTICLE_GAS_CLOUD,
    PARTICLE_BLOOD,
    PARTICLE_DEBRIS,
} ParticleType;

typedef struct Particle {
    bool active;
    ParticleType type;
    Vec2 position;
    Vec2 velocity;
    Vec4 color;
    float size;
    float life;          /* remaining lifetime in seconds */
    float max_life;
    float fade;          /* 0.0 = no fade, 1.0 = full fade over lifetime */
    float gravity;       /* downward acceleration (for debris) */
    float size_decay;    /* shrink rate per second */
} Particle;

typedef struct ParticleSystem {
    Particle particles[MAX_PARTICLES];
    int count;
} ParticleSystem;

void particle_system_init(ParticleSystem *ps);
void particle_update(ParticleSystem *ps, float dt);
void particle_render(ParticleSystem *ps, SpriteBatch *batch, TextureRegion *soft_circle);

/* Spawn pre-configured particle effects */
void particle_spawn_explosion(ParticleSystem *ps, Vec2 position, float radius);
void particle_spawn_smoke(ParticleSystem *ps, Vec2 position, int count);
void particle_spawn_muzzle_flash(ParticleSystem *ps, Vec2 position);
void particle_spawn_gas_cloud(ParticleSystem *ps, Vec2 position, float radius);
void particle_spawn_blood(ParticleSystem *ps, Vec2 position);

/* Spawn a flame stream from origin toward target (cone of fire particles) */
void particle_spawn_flame_stream(ParticleSystem *ps, Vec2 origin, Vec2 target);

#endif /* TD_PARTICLES_H */
