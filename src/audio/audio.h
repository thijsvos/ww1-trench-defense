#ifndef TD_AUDIO_H
#define TD_AUDIO_H

#include <stdbool.h>
#include <stdint.h>
#include "../math/math_types.h"

/* ── Sound IDs ─────────────────────────────────────────────────── */

typedef enum SoundID {
    /* Tower fire */
    SFX_FIRE_MACHINE_GUN = 0,
    SFX_FIRE_MORTAR,
    SFX_FIRE_SNIPER,
    SFX_FIRE_ARTILLERY,
    SFX_FIRE_GAS,
    SFX_FIRE_FLAMETHROWER,

    /* Combat */
    SFX_EXPLOSION_SMALL,
    SFX_EXPLOSION_LARGE,
    SFX_ENEMY_DEATH,
    SFX_ENEMY_ESCAPE,

    /* Economy / building */
    SFX_TOWER_PLACE,
    SFX_TOWER_UPGRADE,
    SFX_TOWER_SELL,
    SFX_GOLD_EARN,

    /* UI */
    SFX_UI_CLICK,

    /* Game events */
    SFX_WAVE_START,
    SFX_WAVE_COMPLETE,
    SFX_VICTORY,
    SFX_DEFEAT,
    SFX_PAUSE,
    SFX_UNPAUSE,

    SFX_COUNT
} SoundID;

typedef enum AmbientID {
    AMBIENT_NONE = -1,
    AMBIENT_BATTLEFIELD = 0,
    AMBIENT_WIND,
    AMBIENT_COUNT
} AmbientID;

typedef enum AudioCategory {
    AUDIO_CAT_COMBAT = 0,
    AUDIO_CAT_UI,
    AUDIO_CAT_EVENT,
    AUDIO_CAT_AMBIENT,
    AUDIO_CAT_COUNT
} AudioCategory;

/* ── Pre-synthesized PCM buffer ────────────────────────────────── */

#define AUDIO_SAMPLE_RATE 22050

typedef struct SoundBuffer {
    int16_t *samples;       /* PCM 16-bit mono */
    int      sample_count;
} SoundBuffer;

/* ── Voice (an active playing instance) ────────────────────────── */

#define MAX_VOICES  32
#define MAX_AMBIENT_VOICES 2

typedef struct Voice {
    bool     active;
    int      sound_id;      /* SoundID or ambient index */
    int      cursor;        /* playback position in samples */
    float    volume;        /* per-instance volume (0..1) */
    float    pan;           /* -1 left, 0 center, 1 right */
    uint8_t  priority;      /* 0 = lowest, 255 = highest */
} Voice;

/* ── Ambient voice (looping with crossfade) ────────────────────── */

typedef struct AmbientVoice {
    bool     active;
    int      ambient_id;
    int      cursor;
    float    volume;        /* current volume (for fade in/out) */
    float    target_volume; /* volume we're fading toward */
    float    fade_speed;    /* volume change per second */
} AmbientVoice;

/* ── Per-sound rate limiting policy ────────────────────────────── */

typedef struct SoundPolicy {
    int      max_concurrent;
    float    cooldown;      /* minimum seconds between triggers */
    float    last_played;   /* time of last play */
    uint8_t  priority;
    AudioCategory category;
} SoundPolicy;

/* ── AudioSystem ───────────────────────────────────────────────── */

typedef struct AudioSystem {
    /* Pre-synthesized buffers */
    SoundBuffer sfx_buffers[SFX_COUNT];
    SoundBuffer ambient_buffers[AMBIENT_COUNT];
    SoundPolicy policies[SFX_COUNT];

    /* Active voices */
    Voice        voices[MAX_VOICES];
    AmbientVoice ambient_voices[MAX_AMBIENT_VOICES];
    AmbientID    current_ambient;

    /* Volume controls */
    float master_volume;
    float category_volume[AUDIO_CAT_COUNT];

    /* Time tracking (monotonic seconds, updated each frame) */
    float time;

    /* miniaudio device (opaque; defined in audio.c) */
    void *device;

    bool initialized;
} AudioSystem;

/* ── Public API ────────────────────────────────────────────────── */

bool audio_init(AudioSystem *audio);
void audio_shutdown(AudioSystem *audio);

/* Call each frame to update cooldown timers and ambient fades */
void audio_update(AudioSystem *audio, float dt);

/* Fire-and-forget SFX */
void audio_play(AudioSystem *audio, SoundID id);
void audio_play_at(AudioSystem *audio, SoundID id,
                   Vec2 world_pos, Vec2 listener_pos);

/* Ambient loops */
void audio_set_ambient(AudioSystem *audio, AmbientID id);
void audio_stop_ambient(AudioSystem *audio);

/* Volume controls */
void audio_set_master_volume(AudioSystem *audio, float vol);
void audio_set_category_volume(AudioSystem *audio, AudioCategory cat, float vol);

#endif /* TD_AUDIO_H */
