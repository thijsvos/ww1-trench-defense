#include "audio.h"
#include "synth.h"

/* ── Sound definitions ─────────────────────────────────────────── *
 * Each SoundDef describes a sound via synthesis parameters.       *
 * Think of this as the audio equivalent of atlas_gen.c.           *
 * ─────────────────────────────────────────────────────────────── */

/* ── SFX Definitions ───────────────────────────────────────────── */

static const SoundDef sfx_defs[SFX_COUNT] = {
    /* SFX_FIRE_MACHINE_GUN — short filtered noise burst, typewriter staccato */
    [SFX_FIRE_MACHINE_GUN] = {
        .base_freq   = 400.0f,
        .freq_slide  = -2000.0f,
        .attack      = 0.002f,
        .sustain     = 0.005f,
        .decay       = 0.015f,
        .wave_noise  = 0.8f,
        .wave_square = 0.2f,
        .lpf_cutoff  = 3000.0f,
        .lpf_sweep   = -8000.0f,
        .volume      = 0.5f,
    },

    /* SFX_FIRE_MORTAR — low thump */
    [SFX_FIRE_MORTAR] = {
        .base_freq   = 100.0f,
        .freq_slide  = -200.0f,
        .attack      = 0.005f,
        .sustain     = 0.03f,
        .decay       = 0.08f,
        .wave_sine   = 0.6f,
        .wave_noise  = 0.4f,
        .lpf_cutoff  = 1500.0f,
        .lpf_sweep   = -3000.0f,
        .volume      = 0.65f,
    },

    /* SFX_FIRE_SNIPER — sharp crack with resonant ping */
    [SFX_FIRE_SNIPER] = {
        .base_freq   = 800.0f,
        .freq_slide  = -3000.0f,
        .attack      = 0.001f,
        .sustain     = 0.01f,
        .decay       = 0.05f,
        .wave_sine   = 0.3f,
        .wave_noise  = 0.7f,
        .hpf_cutoff  = 500.0f,
        .lpf_cutoff  = 6000.0f,
        .lpf_sweep   = -10000.0f,
        .volume      = 0.6f,
    },

    /* SFX_FIRE_ARTILLERY — deep heavy boom */
    [SFX_FIRE_ARTILLERY] = {
        .base_freq   = 60.0f,
        .freq_slide  = -50.0f,
        .attack      = 0.005f,
        .sustain     = 0.06f,
        .decay       = 0.16f,
        .wave_sine   = 0.5f,
        .wave_noise  = 0.6f,
        .lpf_cutoff  = 2000.0f,
        .lpf_sweep   = -4000.0f,
        .volume      = 0.75f,
    },

    /* SFX_FIRE_GAS — sinister high-pass hiss */
    [SFX_FIRE_GAS] = {
        .base_freq   = 0.0f,
        .attack      = 0.05f,
        .sustain     = 0.3f,
        .decay       = 0.15f,
        .wave_noise  = 1.0f,
        .hpf_cutoff  = 2000.0f,
        .hpf_sweep   = 1000.0f,
        .lpf_cutoff  = 6000.0f,
        .volume      = 0.35f,
    },

    /* SFX_FIRE_FLAMETHROWER — pressurized ignition roar (noise only, no pitch) */
    [SFX_FIRE_FLAMETHROWER] = {
        .base_freq   = 0.0f,     /* no tonal component at all */
        .attack      = 0.005f,   /* fast ignition */
        .sustain     = 0.08f,
        .decay       = 0.12f,    /* flame trails off */
        .wave_noise  = 1.0f,     /* pure rushing noise */
        .lpf_cutoff  = 5000.0f,  /* bright sizzle */
        .lpf_sweep   = -3000.0f, /* sweeps down as flame dissipates */
        .hpf_cutoff  = 400.0f,   /* cut the low rumble */
        .volume      = 0.50f,
    },

    /* SFX_EXPLOSION_SMALL — mid noise burst with filter sweep */
    [SFX_EXPLOSION_SMALL] = {
        .base_freq   = 150.0f,
        .freq_slide  = -300.0f,
        .attack      = 0.003f,
        .sustain     = 0.06f,
        .decay       = 0.24f,
        .wave_sine   = 0.3f,
        .wave_noise  = 0.8f,
        .lpf_cutoff  = 4000.0f,
        .lpf_sweep   = -10000.0f,
        .volume      = 0.6f,
    },

    /* SFX_EXPLOSION_LARGE — heavy low boom with sub-bass */
    [SFX_EXPLOSION_LARGE] = {
        .base_freq   = 50.0f,
        .freq_slide  = -80.0f,
        .attack      = 0.005f,
        .sustain     = 0.1f,
        .decay       = 0.4f,
        .wave_sine   = 0.5f,
        .wave_noise  = 0.7f,
        .lpf_cutoff  = 3000.0f,
        .lpf_sweep   = -5000.0f,
        .volume      = 0.75f,
    },

    /* SFX_ENEMY_DEATH — short thud with filtered noise */
    [SFX_ENEMY_DEATH] = {
        .base_freq   = 200.0f,
        .freq_slide  = -1500.0f,
        .attack      = 0.002f,
        .sustain     = 0.015f,
        .decay       = 0.06f,
        .wave_sine   = 0.4f,
        .wave_noise  = 0.5f,
        .lpf_cutoff  = 2500.0f,
        .lpf_sweep   = -6000.0f,
        .volume      = 0.4f,
    },

    /* SFX_ENEMY_ESCAPE — warning descending tone */
    [SFX_ENEMY_ESCAPE] = {
        .base_freq   = 600.0f,
        .freq_slide  = -1500.0f,
        .attack      = 0.005f,
        .sustain     = 0.08f,
        .decay       = 0.12f,
        .wave_sine   = 0.6f,
        .wave_square = 0.3f,
        .lpf_cutoff  = 4000.0f,
        .volume      = 0.5f,
    },

    /* SFX_TOWER_PLACE — satisfying low chunk */
    [SFX_TOWER_PLACE] = {
        .base_freq   = 180.0f,
        .freq_slide  = -400.0f,
        .attack      = 0.001f,
        .sustain     = 0.01f,
        .decay       = 0.025f,
        .wave_square = 0.6f,
        .wave_noise  = 0.3f,
        .lpf_cutoff  = 2000.0f,
        .volume      = 0.55f,
    },

    /* SFX_TOWER_UPGRADE — ascending two-tone chime */
    [SFX_TOWER_UPGRADE] = {
        .base_freq   = 500.0f,
        .freq_slide  = 1500.0f,
        .attack      = 0.005f,
        .sustain     = 0.06f,
        .decay       = 0.09f,
        .wave_sine   = 0.7f,
        .wave_triangle = 0.3f,
        .lpf_cutoff  = 5000.0f,
        .volume      = 0.5f,
    },

    /* SFX_TOWER_SELL — coin/cash high sine ping */
    [SFX_TOWER_SELL] = {
        .base_freq   = 1200.0f,
        .freq_slide  = -400.0f,
        .attack      = 0.001f,
        .sustain     = 0.03f,
        .decay       = 0.07f,
        .wave_sine   = 0.8f,
        .wave_triangle = 0.2f,
        .volume      = 0.45f,
    },

    /* SFX_GOLD_EARN — quick ascending sine */
    [SFX_GOLD_EARN] = {
        .base_freq   = 600.0f,
        .freq_slide  = 3000.0f,
        .attack      = 0.002f,
        .sustain     = 0.03f,
        .decay       = 0.07f,
        .wave_sine   = 0.8f,
        .wave_triangle = 0.2f,
        .lpf_cutoff  = 6000.0f,
        .volume      = 0.35f,
    },

    /* SFX_UI_CLICK — very short sine ping */
    [SFX_UI_CLICK] = {
        .base_freq   = 1000.0f,
        .freq_slide  = -500.0f,
        .attack      = 0.001f,
        .sustain     = 0.005f,
        .decay       = 0.015f,
        .wave_sine   = 0.9f,
        .volume      = 0.35f,
    },

    /* SFX_WAVE_START — WW1 trench whistle (pea whistle / Acme Thunderer) */
    [SFX_WAVE_START] = {
        .base_freq   = 3200.0f,  /* high metallic pitch */
        .freq_slide  = 200.0f,
        .freq_vibrato = 18.0f,   /* fast trill — pea rattling inside */
        .vibrato_depth = 120.0f, /* wide warble, distinctive trill */
        .attack      = 0.005f,   /* near-instant, sharp blast */
        .sustain     = 0.30f,    /* held */
        .decay       = 0.12f,
        .wave_square = 0.55f,    /* harsh metallic overtones */
        .wave_sine   = 0.30f,    /* core pitch */
        .wave_noise  = 0.04f,    /* minimal air texture */
        .wave_saw    = 0.10f,    /* extra bite */
        .hpf_cutoff  = 2000.0f,  /* all highs, no body */
        .lpf_cutoff  = 8000.0f,
        .volume      = 0.40f,
    },

    /* SFX_WAVE_COMPLETE — short trench whistle, slightly lower/shorter than wave start */
    [SFX_WAVE_COMPLETE] = {
        .base_freq   = 2900.0f,
        .freq_slide  = -200.0f,  /* slight descend — "stand down" feel */
        .freq_vibrato = 18.0f,
        .vibrato_depth = 100.0f,
        .attack      = 0.005f,
        .sustain     = 0.15f,    /* shorter blast than wave start */
        .decay       = 0.08f,
        .wave_square = 0.55f,
        .wave_sine   = 0.30f,
        .wave_noise  = 0.04f,
        .wave_saw    = 0.10f,
        .hpf_cutoff  = 2000.0f,
        .lpf_cutoff  = 8000.0f,
        .volume      = 0.35f,
    },

    /* SFX_VICTORY — major chord arpeggio ascending */
    [SFX_VICTORY] = {
        .base_freq   = 440.0f,
        .freq_slide  = 500.0f,
        .freq_vibrato = 4.0f,
        .vibrato_depth = 8.0f,
        .attack      = 0.01f,
        .sustain     = 0.25f,
        .decay       = 0.35f,
        .wave_sine   = 0.6f,
        .wave_triangle = 0.3f,
        .wave_square = 0.1f,
        .lpf_cutoff  = 5000.0f,
        .volume      = 0.6f,
    },

    /* SFX_DEFEAT — descending minor tones */
    [SFX_DEFEAT] = {
        .base_freq   = 440.0f,
        .freq_slide  = -300.0f,
        .freq_vibrato = 3.0f,
        .vibrato_depth = 10.0f,
        .attack      = 0.01f,
        .sustain     = 0.35f,
        .decay       = 0.55f,
        .wave_sine   = 0.5f,
        .wave_saw    = 0.3f,
        .wave_triangle = 0.2f,
        .lpf_cutoff  = 3000.0f,
        .lpf_sweep   = -2000.0f,
        .volume      = 0.55f,
    },

    /* SFX_PAUSE — low tone down */
    [SFX_PAUSE] = {
        .base_freq   = 400.0f,
        .freq_slide  = -800.0f,
        .attack      = 0.005f,
        .sustain     = 0.03f,
        .decay       = 0.07f,
        .wave_sine   = 0.7f,
        .wave_square = 0.2f,
        .lpf_cutoff  = 3000.0f,
        .volume      = 0.4f,
    },

    /* SFX_UNPAUSE — low tone up */
    [SFX_UNPAUSE] = {
        .base_freq   = 300.0f,
        .freq_slide  = 1000.0f,
        .attack      = 0.005f,
        .sustain     = 0.03f,
        .decay       = 0.07f,
        .wave_sine   = 0.7f,
        .wave_square = 0.2f,
        .lpf_cutoff  = 3000.0f,
        .volume      = 0.4f,
    },
};

/* ── Ambient Loop Definitions ──────────────────────────────────── */

static const AmbientDef ambient_defs[AMBIENT_COUNT] = {
    /* AMBIENT_BATTLEFIELD — deep distant artillery rumble, no hiss */
    [AMBIENT_BATTLEFIELD] = {
        .base = {
            .base_freq   = 25.0f,
            .freq_slide  = 0.0f,
            .wave_sine   = 0.6f,   /* deep sub-bass rumble body */
            .wave_noise  = 0.25f,  /* just a touch of texture */
            .wave_triangle = 0.15f,/* slight harmonic variation */
            .lpf_cutoff  = 80.0f,  /* aggressive — kill all hiss */
            .hpf_cutoff  = 12.0f,
            .volume      = 0.55f,
            .sustain     = 6.0f,
        },
        .loop_seconds = 6.0f,
        .crossfade    = 0.8f,
    },

    /* AMBIENT_WIND — hollow moaning wind, no-man's-land */
    [AMBIENT_WIND] = {
        .base = {
            .base_freq   = 55.0f,  /* low tonal body */
            .freq_vibrato = 0.3f,  /* slow pitch drift */
            .vibrato_depth = 15.0f,
            .wave_sine   = 0.35f,  /* tonal moan */
            .wave_noise  = 0.45f,  /* wind texture */
            .wave_triangle = 0.1f,
            .lpf_cutoff  = 250.0f, /* tight — removes hiss, keeps moan */
            .hpf_cutoff  = 40.0f,
            .volume      = 0.40f,
            .sustain     = 5.0f,
        },
        .loop_seconds = 5.0f,
        .crossfade    = 0.6f,
    },

    /* AMBIENT_GALLIPOLI — airy coastal rumble, Gallipoli beach landing */
    [AMBIENT_GALLIPOLI] = {
        .base = {
            .base_freq     = 30.0f,   /* slightly less deep than battlefield */
            .freq_vibrato  = 0.15f,   /* slow drift, irregular surf/wind */
            .vibrato_depth = 5.0f,
            .wave_sine     = 0.45f,   /* less dominant sine body */
            .wave_noise    = 0.35f,   /* more texture, wind across the beach */
            .wave_triangle = 0.10f,
            .lpf_cutoff    = 120.0f,  /* opens up for airy quality */
            .hpf_cutoff    = 12.0f,
            .volume        = 0.50f,   /* slightly quieter, less oppressive */
            .sustain       = 6.0f,
        },
        .loop_seconds = 6.0f,
        .crossfade    = 0.8f,
    },

    /* AMBIENT_VERDUN — heavy oppressive grinding bass, Verdun meatgrinder */
    [AMBIENT_VERDUN] = {
        .base = {
            .base_freq     = 20.0f,   /* deeper, heavier thudding */
            .wave_sine     = 0.70f,   /* more tonal weight, grinding bass */
            .wave_noise    = 0.20f,   /* less texture, pure oppressive tone */
            .wave_triangle = 0.10f,
            .lpf_cutoff    = 60.0f,   /* tighter filter, suffocating and dense */
            .hpf_cutoff    = 10.0f,   /* allows more sub-bass through */
            .volume        = 0.60f,   /* louder, more present */
            .sustain       = 6.0f,
        },
        .loop_seconds = 6.0f,
        .crossfade    = 0.8f,
    },

    /* AMBIENT_BRUSILOV — open windswept plains, Eastern Front steppes */
    [AMBIENT_BRUSILOV] = {
        .base = {
            .base_freq     = 40.0f,   /* higher, more wind than artillery */
            .freq_vibrato  = 0.25f,   /* gusting wind across open terrain */
            .vibrato_depth = 10.0f,
            .wave_sine     = 0.40f,   /* balanced with noise */
            .wave_noise    = 0.40f,   /* wind texture dominates equally */
            .wave_triangle = 0.10f,
            .lpf_cutoff    = 180.0f,  /* open, letting wind through */
            .hpf_cutoff    = 18.0f,   /* reduces deep rumble */
            .volume        = 0.45f,   /* quieter, more atmospheric */
            .sustain       = 7.0f,
        },
        .loop_seconds = 7.0f,
        .crossfade    = 0.9f,
    },

    /* AMBIENT_KAISERSCHLACHT — full climactic battlefield, the final push */
    [AMBIENT_KAISERSCHLACHT] = {
        .base = {
            .base_freq     = 25.0f,   /* same deep foundation as baseline */
            .freq_vibrato  = 0.10f,   /* subtle movement, constant bombardment */
            .vibrato_depth = 4.0f,
            .wave_sine     = 0.55f,
            .wave_noise    = 0.30f,   /* more noise for chaotic texture */
            .wave_triangle = 0.15f,
            .lpf_cutoff    = 100.0f,  /* opens slightly for richer sound */
            .hpf_cutoff    = 10.0f,   /* more sub-bass */
            .volume        = 0.62f,   /* loudest ambient, climactic */
            .sustain       = 6.0f,
        },
        .loop_seconds = 6.0f,
        .crossfade    = 0.8f,
    },
};

/* ── Per-sound policies ────────────────────────────────────────── */

static const SoundPolicy default_policies[SFX_COUNT] = {
    [SFX_FIRE_MACHINE_GUN] = { .max_concurrent = 3,  .cooldown = 0.08f,  .priority = 60,  .category = AUDIO_CAT_COMBAT },
    [SFX_FIRE_MORTAR]      = { .max_concurrent = 2,  .cooldown = 0.20f,  .priority = 80,  .category = AUDIO_CAT_COMBAT },
    [SFX_FIRE_SNIPER]      = { .max_concurrent = 2,  .cooldown = 0.15f,  .priority = 90,  .category = AUDIO_CAT_COMBAT },
    [SFX_FIRE_ARTILLERY]   = { .max_concurrent = 1,  .cooldown = 0.50f,  .priority = 100, .category = AUDIO_CAT_COMBAT },
    [SFX_FIRE_GAS]         = { .max_concurrent = 2,  .cooldown = 0.30f,  .priority = 50,  .category = AUDIO_CAT_COMBAT },
    [SFX_FIRE_FLAMETHROWER]= { .max_concurrent = 2,  .cooldown = 0.10f,  .priority = 70,  .category = AUDIO_CAT_COMBAT },
    [SFX_EXPLOSION_SMALL]  = { .max_concurrent = 3,  .cooldown = 0.10f,  .priority = 85,  .category = AUDIO_CAT_COMBAT },
    [SFX_EXPLOSION_LARGE]  = { .max_concurrent = 2,  .cooldown = 0.30f,  .priority = 95,  .category = AUDIO_CAT_COMBAT },
    [SFX_ENEMY_DEATH]      = { .max_concurrent = 4,  .cooldown = 0.05f,  .priority = 40,  .category = AUDIO_CAT_COMBAT },
    [SFX_ENEMY_ESCAPE]     = { .max_concurrent = 1,  .cooldown = 0.50f,  .priority = 150, .category = AUDIO_CAT_EVENT },
    [SFX_TOWER_PLACE]      = { .max_concurrent = 1,  .cooldown = 0.0f,   .priority = 200, .category = AUDIO_CAT_UI },
    [SFX_TOWER_UPGRADE]    = { .max_concurrent = 1,  .cooldown = 0.0f,   .priority = 200, .category = AUDIO_CAT_UI },
    [SFX_TOWER_SELL]       = { .max_concurrent = 1,  .cooldown = 0.0f,   .priority = 200, .category = AUDIO_CAT_UI },
    [SFX_GOLD_EARN]        = { .max_concurrent = 2,  .cooldown = 0.05f,  .priority = 30,  .category = AUDIO_CAT_UI },
    [SFX_UI_CLICK]         = { .max_concurrent = 1,  .cooldown = 0.0f,   .priority = 250, .category = AUDIO_CAT_UI },
    [SFX_WAVE_START]       = { .max_concurrent = 1,  .cooldown = 0.0f,   .priority = 255, .category = AUDIO_CAT_EVENT },
    [SFX_WAVE_COMPLETE]    = { .max_concurrent = 1,  .cooldown = 0.0f,   .priority = 255, .category = AUDIO_CAT_EVENT },
    [SFX_VICTORY]          = { .max_concurrent = 1,  .cooldown = 0.0f,   .priority = 255, .category = AUDIO_CAT_EVENT },
    [SFX_DEFEAT]           = { .max_concurrent = 1,  .cooldown = 0.0f,   .priority = 255, .category = AUDIO_CAT_EVENT },
    [SFX_PAUSE]            = { .max_concurrent = 1,  .cooldown = 0.0f,   .priority = 255, .category = AUDIO_CAT_UI },
    [SFX_UNPAUSE]          = { .max_concurrent = 1,  .cooldown = 0.0f,   .priority = 255, .category = AUDIO_CAT_UI },
};

/* ── Initialization (called by audio_init) ─────────────────────── */

void sound_defs_init(AudioSystem *audio)
{
    /* Synthesize all SFX */
    for (int i = 0; i < SFX_COUNT; i++)
        synth_generate(&sfx_defs[i], &audio->sfx_buffers[i]);

    /* Synthesize ambient loops */
    for (int i = 0; i < AMBIENT_COUNT; i++)
        synth_generate_loop(&ambient_defs[i], &audio->ambient_buffers[i]);

    /* Copy policies */
    for (int i = 0; i < SFX_COUNT; i++)
        audio->policies[i] = default_policies[i];
}
