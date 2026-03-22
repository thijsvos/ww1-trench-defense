#include "particles.h"
#include "../math/vec.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ------------------------------------------------------------------ */
/*  Random helpers                                                     */
/* ------------------------------------------------------------------ */

static inline float randf(void)
{
    return (float)rand() / (float)RAND_MAX;
}

static inline float randf_range(float min, float max)
{
    return min + randf() * (max - min);
}

/* ------------------------------------------------------------------ */
/*  Y layer for particles (above projectiles, below debug)             */
/* ------------------------------------------------------------------ */

#define PARTICLE_Y 0.05f

/* ------------------------------------------------------------------ */
/*  Init                                                               */
/* ------------------------------------------------------------------ */

void particle_system_init(ParticleSystem *ps)
{
    memset(ps, 0, sizeof(*ps));
}

/* ------------------------------------------------------------------ */
/*  Internal: add a single particle                                    */
/* ------------------------------------------------------------------ */

static Particle *particle_add(ParticleSystem *ps)
{
    if (ps->count >= MAX_PARTICLES) return NULL;

    Particle *p = &ps->particles[ps->count];
    memset(p, 0, sizeof(*p));
    p->active = true;
    ps->count++;
    return p;
}

/* ------------------------------------------------------------------ */
/*  Update                                                             */
/* ------------------------------------------------------------------ */

void particle_update(ParticleSystem *ps, float dt)
{
    int i = 0;

    while (i < ps->count) {
        Particle *p = &ps->particles[i];

        /* decrement lifetime */
        p->life -= dt;

        if (p->life <= 0.0f) {
            /* swap-on-remove: replace with last */
            ps->particles[i] = ps->particles[ps->count - 1];
            ps->count--;
            continue; /* re-check this index */
        }

        /* apply gravity (positive gravity pulls downward on the Z/Y render axis) */
        p->velocity.y -= p->gravity * dt;

        /* integrate position */
        p->position = vec2_add(p->position, vec2_scale(p->velocity, dt));

        /* apply fade: alpha decreases linearly over lifetime */
        if (p->fade > 0.0f) {
            float life_ratio = p->life / p->max_life;
            p->color.w = life_ratio * p->fade + (1.0f - p->fade) * p->color.w;
        }

        /* apply size decay */
        if (p->size_decay > 0.0f) {
            p->size -= p->size_decay * dt;
            if (p->size < 0.01f) p->size = 0.01f;
        }

        i++;
    }
}

/* ------------------------------------------------------------------ */
/*  Render                                                             */
/* ------------------------------------------------------------------ */

void particle_render(ParticleSystem *ps, SpriteBatch *batch, TextureRegion *soft_circle)
{
    for (int i = 0; i < ps->count; i++) {
        Particle *p = &ps->particles[i];

        Vec3 pos = vec3(p->position.x, PARTICLE_Y, p->position.y);
        Vec2 size = vec2(p->size, p->size);

        if ((p->type == PARTICLE_GAS_CLOUD || p->type == PARTICLE_SMOKE) && soft_circle) {
            /* Render gas/smoke as soft circles for cloud-like appearance */
            sprite_batch_draw_textured(batch, pos, size, soft_circle, p->color);
        } else {
            sprite_batch_draw_quad(batch, pos, size, p->color);
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Explosion: 12-20 particles, orange/red, fast outward               */
/* ------------------------------------------------------------------ */

void particle_spawn_explosion(ParticleSystem *ps, Vec2 position, float radius)
{
    int count = 12 + (rand() % 9); /* 12 to 20 */

    for (int i = 0; i < count; i++) {
        Particle *p = particle_add(ps);
        if (!p) return;

        float angle = randf() * 2.0f * (float)M_PI;
        float speed = randf_range(1.0f, 3.0f);
        float offset = randf() * radius * 0.3f;

        p->position = vec2_add(position, vec2(cosf(angle) * offset, sinf(angle) * offset));
        p->velocity = vec2(cosf(angle) * speed, sinf(angle) * speed);

        /* orange to red gradient */
        float r = randf_range(0.8f, 1.0f);
        float g = randf_range(0.2f, 0.6f);
        p->color = vec4(r, g, 0.0f, 1.0f);

        p->size       = randf_range(0.08f, 0.15f) * (1.0f + radius * 0.3f);
        p->life       = randf_range(0.3f, 0.6f);
        p->max_life   = p->life;
        p->fade       = 1.0f;
        p->gravity    = 0.0f;
        p->size_decay = p->size * 1.5f; /* shrink to nothing over life */
    }
}

/* ------------------------------------------------------------------ */
/*  Smoke: gray, slow upward drift                                     */
/* ------------------------------------------------------------------ */

void particle_spawn_smoke(ParticleSystem *ps, Vec2 position, int count)
{
    for (int i = 0; i < count; i++) {
        Particle *p = particle_add(ps);
        if (!p) return;

        p->type = PARTICLE_SMOKE;
        float spread = randf_range(-0.15f, 0.15f);
        p->position = vec2_add(position, vec2(spread, spread));

        /* slow upward drift with slight random horizontal */
        p->velocity = vec2(randf_range(-0.2f, 0.2f), randf_range(0.1f, 0.4f));

        float gray = randf_range(0.4f, 0.6f);
        p->color = vec4(gray, gray, gray, randf_range(0.3f, 0.5f));

        p->size       = randf_range(0.1f, 0.2f);
        p->life       = randf_range(0.5f, 1.0f);
        p->max_life   = p->life;
        p->fade       = 1.0f;
        p->gravity    = 0.0f;
        p->size_decay = 0.0f; /* smoke doesn't shrink, it fades */
    }
}

/* ------------------------------------------------------------------ */
/*  Muzzle flash: 3-5 bright yellow/white, very fast, very short       */
/* ------------------------------------------------------------------ */

void particle_spawn_muzzle_flash(ParticleSystem *ps, Vec2 position)
{
    int count = 3 + (rand() % 3); /* 3 to 5 */

    for (int i = 0; i < count; i++) {
        Particle *p = particle_add(ps);
        if (!p) return;

        float angle = randf() * 2.0f * (float)M_PI;
        float speed = randf_range(3.0f, 6.0f);

        p->position = position;
        p->velocity = vec2(cosf(angle) * speed, sinf(angle) * speed);

        /* bright yellow to white */
        float wb = randf_range(0.7f, 1.0f);
        p->color = vec4(1.0f, 1.0f, wb, 1.0f);

        p->size       = randf_range(0.04f, 0.08f);
        p->life       = randf_range(0.05f, 0.1f);
        p->max_life   = p->life;
        p->fade       = 1.0f;
        p->gravity    = 0.0f;
        p->size_decay = 0.0f;
    }
}

/* ------------------------------------------------------------------ */
/*  Gas cloud: mustard yellow, slow random drift, long-lived           */
/* ------------------------------------------------------------------ */

void particle_spawn_gas_cloud(ParticleSystem *ps, Vec2 position, float radius)
{
    int count = 12 + (rand() % 6); /* 12 to 17 */

    for (int i = 0; i < count; i++) {
        Particle *p = particle_add(ps);
        if (!p) return;

        /* spawn within the radius */
        float angle  = randf() * 2.0f * (float)M_PI;
        float dist   = randf() * radius;

        p->position = vec2_add(position, vec2(cosf(angle) * dist, sinf(angle) * dist));

        /* slow expanding drift outward from center */
        float drift_speed = randf_range(0.05f, 0.2f);
        p->velocity = vec2(cosf(angle) * drift_speed, sinf(angle) * drift_speed);

        p->type = PARTICLE_GAS_CLOUD;

        /* mustard yellow / sickly green, semi-transparent */
        float green_shift = randf_range(0.0f, 0.15f);
        p->color = vec4(
            randf_range(0.55f, 0.75f),
            randf_range(0.58f, 0.72f) + green_shift,
            randf_range(0.05f, 0.18f),
            randf_range(0.3f, 0.5f)
        );

        p->size       = randf_range(0.25f, 0.5f);
        p->life       = randf_range(2.0f, 4.0f);
        p->max_life   = p->life;
        p->fade       = 1.0f;
        p->gravity    = 0.0f;
        p->size_decay = 0.01f; /* very slow shrink — cloud lingers */
    }
}

/* ------------------------------------------------------------------ */
/*  Blood: small dark red particles, fast outward burst                */
/* ------------------------------------------------------------------ */

void particle_spawn_blood(ParticleSystem *ps, Vec2 position)
{
    int count = 4 + (rand() % 5); /* 4 to 8 */

    for (int i = 0; i < count; i++) {
        Particle *p = particle_add(ps);
        if (!p) return;

        float angle = randf() * 2.0f * (float)M_PI;
        float speed = randf_range(1.0f, 2.5f);

        p->position = position;
        p->velocity = vec2(cosf(angle) * speed, sinf(angle) * speed);

        /* dark red */
        p->color = vec4(
            randf_range(0.4f, 0.6f),
            randf_range(0.0f, 0.05f),
            randf_range(0.0f, 0.05f),
            1.0f
        );

        p->size       = randf_range(0.03f, 0.06f);
        p->life       = randf_range(0.2f, 0.4f);
        p->max_life   = p->life;
        p->fade       = 1.0f;
        p->gravity    = 2.0f; /* blood drops fall */
        p->size_decay = 0.0f;
    }
}

/* ------------------------------------------------------------------ */
/*  Flame stream: cone of fire particles from origin to target         */
/* ------------------------------------------------------------------ */

void particle_spawn_flame_stream(ParticleSystem *ps, Vec2 origin, Vec2 target)
{
    Vec2 dir = vec2_sub(target, origin);
    float dist = vec2_length(dir);
    if (dist < 0.01f) return;
    Vec2 norm = vec2_scale(dir, 1.0f / dist);
    /* perpendicular for cone spread */
    Vec2 perp = vec2(norm.y, -norm.x);

    int count = 10 + (rand() % 6); /* 10-15 particles per burst */

    for (int i = 0; i < count; i++) {
        Particle *p = particle_add(ps);
        if (!p) return;

        /* Place particles along the stream from origin toward target */
        float t = randf_range(0.1f, 1.0f); /* how far along the stream */
        float spread = t * randf_range(-0.4f, 0.4f); /* wider spread further out */

        p->position = vec2(
            origin.x + norm.x * dist * t + perp.x * spread,
            origin.y + norm.y * dist * t + perp.y * spread
        );

        /* Velocity: mostly forward with some spread */
        float speed = randf_range(1.5f, 4.0f);
        float side_speed = randf_range(-0.8f, 0.8f);
        p->velocity = vec2(
            norm.x * speed + perp.x * side_speed,
            norm.y * speed + perp.y * side_speed
        );

        /* Color: bright yellow core → orange → dark red at tips */
        if (t < 0.3f) {
            /* Near tower: bright yellow-white */
            p->color = vec4(1.0f, randf_range(0.85f, 1.0f), randf_range(0.3f, 0.6f), 1.0f);
        } else if (t < 0.6f) {
            /* Middle: orange */
            p->color = vec4(1.0f, randf_range(0.45f, 0.65f), randf_range(0.0f, 0.15f), 0.9f);
        } else {
            /* Tips: dark red / smoky */
            p->color = vec4(randf_range(0.6f, 0.9f), randf_range(0.15f, 0.3f), 0.0f, 0.7f);
        }

        p->size       = randf_range(0.1f, 0.25f) * (0.5f + t); /* bigger further out */
        p->life       = randf_range(0.15f, 0.35f);
        p->max_life   = p->life;
        p->fade       = 1.0f;
        p->gravity    = 0.0f;
        p->size_decay = 0.3f;
    }

    /* Add a few smoke particles at the end of the stream */
    for (int i = 0; i < 3; i++) {
        Particle *p = particle_add(ps);
        if (!p) return;

        p->position = vec2(
            target.x + randf_range(-0.2f, 0.2f),
            target.y + randf_range(-0.2f, 0.2f)
        );
        p->velocity = vec2(randf_range(-0.3f, 0.3f), randf_range(-0.5f, -0.1f));
        p->color = vec4(0.3f, 0.3f, 0.3f, 0.4f);
        p->size = randf_range(0.15f, 0.25f);
        p->life = randf_range(0.4f, 0.8f);
        p->max_life = p->life;
        p->fade = 1.0f;
        p->gravity = 0.0f;
        p->size_decay = 0.05f;
    }
}
