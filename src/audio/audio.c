#include "audio.h"
#include "../core/log.h"
#include "miniaudio.h"
#include "synth.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ── Forward declarations ──────────────────────────────────────── */

static void audio_data_callback(ma_device *device, void *output,
                                const void *input, ma_uint32 frame_count);
static int  find_free_voice(AudioSystem *audio, uint8_t priority);
static int  count_active_sound(AudioSystem *audio, int sound_id);

/* Defined in sound_defs.c */
extern void sound_defs_init(AudioSystem *audio);

/* ── Init / Shutdown ───────────────────────────────────────────── */

bool audio_init(AudioSystem *audio)
{
    memset(audio, 0, sizeof(*audio));

    audio->master_volume = 0.7f;
    for (int i = 0; i < AUDIO_CAT_COUNT; i++)
        audio->category_volume[i] = 1.0f;
    audio->current_ambient = AMBIENT_NONE;

    /* Synthesize all sound buffers */
    sound_defs_init(audio);

    /* Create miniaudio device */
    ma_device *device = malloc(sizeof(ma_device));
    if (!device) {
        LOG_ERROR("Failed to allocate audio device");
        return false;
    }

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format   = ma_format_s16;
    config.playback.channels = 1;
    config.sampleRate        = AUDIO_SAMPLE_RATE;
    config.dataCallback      = audio_data_callback;
    config.pUserData         = audio;

    if (ma_device_init(NULL, &config, device) != MA_SUCCESS) {
        LOG_ERROR("Failed to init audio device");
        free(device);
        return false;
    }

    if (ma_device_start(device) != MA_SUCCESS) {
        LOG_ERROR("Failed to start audio device");
        ma_device_uninit(device);
        free(device);
        return false;
    }

    audio->device = device;
    audio->initialized = true;
    LOG_INFO("Audio initialized: %d Hz, mono, 16-bit", AUDIO_SAMPLE_RATE);
    return true;
}

void audio_shutdown(AudioSystem *audio)
{
    if (!audio->initialized) return;

    ma_device *device = (ma_device *)audio->device;
    ma_device_uninit(device);
    free(device);

    /* Free synthesized buffers */
    for (int i = 0; i < SFX_COUNT; i++)
        free(audio->sfx_buffers[i].samples);
    for (int i = 0; i < AMBIENT_COUNT; i++)
        free(audio->ambient_buffers[i].samples);

    audio->initialized = false;
    LOG_INFO("Audio shut down");
}

/* ── Frame update ──────────────────────────────────────────────── */

void audio_update(AudioSystem *audio, float dt)
{
    if (!audio->initialized) return;
    audio->time += dt;

    /* Update ambient voice fades */
    for (int i = 0; i < MAX_AMBIENT_VOICES; i++) {
        AmbientVoice *av = &audio->ambient_voices[i];
        if (!av->active) continue;

        if (av->volume < av->target_volume) {
            av->volume += av->fade_speed * dt;
            if (av->volume >= av->target_volume)
                av->volume = av->target_volume;
        } else if (av->volume > av->target_volume) {
            av->volume -= av->fade_speed * dt;
            if (av->volume <= av->target_volume) {
                av->volume = av->target_volume;
                /* If faded to silence, deactivate */
                if (av->target_volume <= 0.0f)
                    av->active = false;
            }
        }
    }
}

/* ── Play SFX ──────────────────────────────────────────────────── */

static void audio_play_internal(AudioSystem *audio, SoundID id, float volume)
{
    if (!audio || !audio->initialized) return;
    if (id < 0 || id >= SFX_COUNT) return;
    if (audio->sfx_buffers[id].sample_count == 0) return;

    SoundPolicy *policy = &audio->policies[id];

    /* Cooldown check */
    float elapsed = audio->time - policy->last_played;
    if (elapsed < policy->cooldown) return;

    /* Max concurrent check */
    if (policy->max_concurrent > 0) {
        if (count_active_sound(audio, id) >= policy->max_concurrent)
            return;
    }

    /* Find a voice slot */
    int slot = find_free_voice(audio, policy->priority);
    if (slot < 0) return;

    /* Activate voice */
    Voice *v = &audio->voices[slot];
    v->active   = true;
    v->sound_id = id;
    v->cursor   = 0;
    v->volume   = volume * audio->category_volume[policy->category]
                         * audio->master_volume;
    v->pan      = 0.0f;
    v->priority = policy->priority;

    policy->last_played = audio->time;
}

void audio_play(AudioSystem *audio, SoundID id)
{
    audio_play_internal(audio, id, 1.0f);
}

void audio_play_at(AudioSystem *audio, SoundID id,
                   Vec2 world_pos, Vec2 listener_pos)
{
    if (!audio || !audio->initialized) return;

    float dx = world_pos.x - listener_pos.x;
    float dy = world_pos.y - listener_pos.y;
    float dist = sqrtf(dx * dx + dy * dy);

    float max_dist = 20.0f; /* roughly 2x viewport radius in world units */
    if (dist > max_dist) return; /* too far, skip */

    float half_dist = max_dist * 0.5f;
    float volume = 1.0f;
    if (dist > half_dist)
        volume = 1.0f - (dist - half_dist) / (max_dist - half_dist) * 0.8f;

    audio_play_internal(audio, id, volume);
}

/* ── Ambient loops ─────────────────────────────────────────────── */

void audio_set_ambient(AudioSystem *audio, AmbientID id)
{
    if (!audio || !audio->initialized) return;
    if (id < 0 || id >= AMBIENT_COUNT) return;
    if (id == audio->current_ambient) return;

    float fade_time = 0.5f; /* 500ms crossfade */

    /* Fade out any currently playing ambient */
    for (int i = 0; i < MAX_AMBIENT_VOICES; i++) {
        AmbientVoice *av = &audio->ambient_voices[i];
        if (av->active && av->target_volume > 0.0f) {
            av->target_volume = 0.0f;
            av->fade_speed = av->volume / fade_time;
        }
    }

    /* Find a free ambient slot and start the new one */
    for (int i = 0; i < MAX_AMBIENT_VOICES; i++) {
        AmbientVoice *av = &audio->ambient_voices[i];
        if (!av->active || av->target_volume <= 0.0f) {
            av->active = true;
            av->ambient_id = id;
            av->cursor = 0;
            av->volume = 0.0f;
            av->target_volume = audio->category_volume[AUDIO_CAT_AMBIENT]
                              * audio->master_volume * 0.8f;
            av->fade_speed = av->target_volume / fade_time;
            break;
        }
    }

    audio->current_ambient = id;
}

void audio_stop_ambient(AudioSystem *audio)
{
    if (!audio || !audio->initialized) return;

    float fade_time = 0.5f;
    for (int i = 0; i < MAX_AMBIENT_VOICES; i++) {
        AmbientVoice *av = &audio->ambient_voices[i];
        if (av->active) {
            av->target_volume = 0.0f;
            av->fade_speed = (av->volume > 0.0f)
                           ? av->volume / fade_time
                           : 1.0f;
        }
    }
    audio->current_ambient = AMBIENT_NONE;
}

/* ── Volume controls ───────────────────────────────────────────── */

void audio_set_master_volume(AudioSystem *audio, float vol)
{
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    audio->master_volume = vol;
}

void audio_set_category_volume(AudioSystem *audio, AudioCategory cat, float vol)
{
    if (cat < 0 || cat >= AUDIO_CAT_COUNT) return;
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    audio->category_volume[cat] = vol;
}

/* ── miniaudio data callback (runs on audio thread) ────────────── */

static void audio_data_callback(ma_device *device, void *output,
                                const void *input, ma_uint32 frame_count)
{
    (void)input;
    AudioSystem *audio = (AudioSystem *)device->pUserData;
    int16_t *out = (int16_t *)output;

    for (ma_uint32 f = 0; f < frame_count; f++) {
        float mixed = 0.0f;

        /* Mix SFX voices */
        for (int i = 0; i < MAX_VOICES; i++) {
            Voice *v = &audio->voices[i];
            if (!v->active) continue;

            SoundBuffer *buf = &audio->sfx_buffers[v->sound_id];
            if (v->cursor >= buf->sample_count) {
                v->active = false;
                continue;
            }

            float sample = (float)buf->samples[v->cursor] / 32768.0f;
            mixed += sample * v->volume;
            v->cursor++;
        }

        /* Mix ambient voices (looping) */
        for (int i = 0; i < MAX_AMBIENT_VOICES; i++) {
            AmbientVoice *av = &audio->ambient_voices[i];
            if (!av->active) continue;
            if (av->ambient_id < 0 || av->ambient_id >= AMBIENT_COUNT) continue;

            SoundBuffer *buf = &audio->ambient_buffers[av->ambient_id];
            if (buf->sample_count == 0) continue;

            float sample = (float)buf->samples[av->cursor] / 32768.0f;
            mixed += sample * av->volume;
            av->cursor++;

            /* Loop seamlessly */
            if (av->cursor >= buf->sample_count)
                av->cursor = 0;
        }

        /* Soft limiter to prevent clipping */
        if (mixed >  0.95f) mixed =  0.95f + (mixed - 0.95f) * 0.1f;
        if (mixed < -0.95f) mixed = -0.95f + (mixed + 0.95f) * 0.1f;

        /* Clamp to int16 range */
        if (mixed >  1.0f) mixed =  1.0f;
        if (mixed < -1.0f) mixed = -1.0f;

        out[f] = (int16_t)(mixed * 32767.0f);
    }
}

/* ── Voice management helpers ──────────────────────────────────── */

static int find_free_voice(AudioSystem *audio, uint8_t priority)
{
    /* First pass: find an inactive slot */
    for (int i = 0; i < MAX_VOICES; i++) {
        if (!audio->voices[i].active)
            return i;
    }

    /* Second pass: steal lowest-priority voice if our priority is higher */
    int lowest_idx = -1;
    uint8_t lowest_pri = 255;
    for (int i = 0; i < MAX_VOICES; i++) {
        if (audio->voices[i].priority < lowest_pri) {
            lowest_pri = audio->voices[i].priority;
            lowest_idx = i;
        }
    }

    if (lowest_idx >= 0 && priority > lowest_pri)
        return lowest_idx;

    return -1;
}

static int count_active_sound(AudioSystem *audio, int sound_id)
{
    int count = 0;
    for (int i = 0; i < MAX_VOICES; i++) {
        if (audio->voices[i].active && audio->voices[i].sound_id == sound_id)
            count++;
    }
    return count;
}
