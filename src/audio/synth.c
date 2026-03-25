#include "synth.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ── Random noise generator (simple LCG) ───────────────────────── */

static uint32_t noise_state = 0xDEADBEEF;

static float noise_sample(void)
{
    noise_state = noise_state * 1103515245 + 12345;
    return ((float)(noise_state >> 16) / 32768.0f) - 1.0f;
}

/* ── Oscillator ────────────────────────────────────────────────── */

static float oscillator(float phase, const SoundDef *def)
{
    float out = 0.0f;
    float p = fmodf(phase, 1.0f);
    if (p < 0.0f) p += 1.0f;

    if (def->wave_sine > 0.0f)
        out += sinf(p * 2.0f * (float)M_PI) * def->wave_sine;

    if (def->wave_square > 0.0f)
        out += (p < 0.5f ? 1.0f : -1.0f) * def->wave_square;

    if (def->wave_saw > 0.0f)
        out += (2.0f * p - 1.0f) * def->wave_saw;

    if (def->wave_triangle > 0.0f)
        out += (4.0f * fabsf(p - 0.5f) - 1.0f) * def->wave_triangle;

    if (def->wave_noise > 0.0f)
        out += noise_sample() * def->wave_noise;

    return out;
}

/* ── ADSR envelope ─────────────────────────────────────────────── */

static float envelope(float t, float attack, float sustain, float decay)
{
    if (t < attack) {
        return (attack > 0.0f) ? t / attack : 1.0f;
    }
    t -= attack;
    if (t < sustain) {
        return 1.0f;
    }
    t -= sustain;
    if (t < decay) {
        return (decay > 0.0f) ? 1.0f - t / decay : 0.0f;
    }
    return 0.0f;
}

/* ── Simple one-pole low-pass filter ───────────────────────────── */

typedef struct FilterState {
    float lp_prev;
    float hp_prev;
    float hp_input_prev;
} FilterState;

static float apply_filters(float sample, const SoundDef *def, float t,
                           FilterState *fs)
{
    float out = sample;

    /* Low-pass filter */
    float lpf = def->lpf_cutoff + def->lpf_sweep * t;
    if (lpf > 0.0f && lpf < (float)AUDIO_SAMPLE_RATE * 0.5f) {
        float rc = 1.0f / (2.0f * (float)M_PI * lpf);
        float dt = 1.0f / (float)AUDIO_SAMPLE_RATE;
        float alpha = dt / (rc + dt);
        fs->lp_prev += alpha * (out - fs->lp_prev);
        out = fs->lp_prev;
    }

    /* High-pass filter */
    float hpf = def->hpf_cutoff + def->hpf_sweep * t;
    if (hpf > 0.0f && hpf < (float)AUDIO_SAMPLE_RATE * 0.5f) {
        float rc = 1.0f / (2.0f * (float)M_PI * hpf);
        float dt = 1.0f / (float)AUDIO_SAMPLE_RATE;
        float alpha = rc / (rc + dt);
        float hp_out = alpha * (fs->hp_prev + out - fs->hp_input_prev);
        fs->hp_input_prev = out;
        fs->hp_prev = hp_out;
        out = hp_out;
    }

    return out;
}

/* ── Generate one-shot PCM buffer ──────────────────────────────── */

void synth_generate(const SoundDef *def, SoundBuffer *out)
{
    float total_time = def->attack + def->sustain + def->decay;
    int sample_count = (int)(total_time * AUDIO_SAMPLE_RATE) + 1;

    out->samples = (int16_t *)malloc(sample_count * sizeof(int16_t));
    out->sample_count = sample_count;
    if (!out->samples) {
        out->sample_count = 0;
        return;
    }

    float phase = 0.0f;
    float dt = 1.0f / (float)AUDIO_SAMPLE_RATE;
    FilterState fs = {0};

    /* Reset noise state for reproducible results */
    noise_state = 0xDEADBEEF;

    for (int i = 0; i < sample_count; i++) {
        float t = (float)i * dt;

        /* Frequency with slide and vibrato */
        float freq = def->base_freq + def->freq_slide * t;
        if (def->freq_vibrato > 0.0f && def->vibrato_depth > 0.0f)
            freq += sinf(t * def->freq_vibrato * 2.0f * (float)M_PI) * def->vibrato_depth;
        if (freq < 0.0f) freq = 0.0f;

        /* Oscillator */
        float sample = oscillator(phase, def);
        phase += freq * dt;

        /* Envelope */
        sample *= envelope(t, def->attack, def->sustain, def->decay);

        /* Filters */
        sample = apply_filters(sample, def, t, &fs);

        /* Volume */
        sample *= def->volume;

        /* Convert to int16 */
        if (sample >  1.0f) sample =  1.0f;
        if (sample < -1.0f) sample = -1.0f;
        out->samples[i] = (int16_t)(sample * 32767.0f);
    }
}

/* ── Generate seamless looping buffer ──────────────────────────── */

void synth_generate_loop(const AmbientDef *def, SoundBuffer *out)
{
    /* Generate a buffer of loop_seconds length */
    SoundDef extended = def->base;
    extended.attack  = 0.0f; /* no envelope for loops */
    extended.sustain = def->loop_seconds;
    extended.decay   = 0.0f;

    int sample_count = (int)(def->loop_seconds * AUDIO_SAMPLE_RATE);

    out->samples = (int16_t *)malloc(sample_count * sizeof(int16_t));
    out->sample_count = sample_count;
    if (!out->samples) {
        out->sample_count = 0;
        return;
    }

    float phase = 0.0f;
    float dt = 1.0f / (float)AUDIO_SAMPLE_RATE;
    FilterState fs = {0};

    noise_state = 0xCAFEBABE; /* different seed for ambient */

    for (int i = 0; i < sample_count; i++) {
        float t = (float)i * dt;

        float freq = extended.base_freq + extended.freq_slide * t;
        if (freq < 0.0f) freq = 0.0f;

        float sample = oscillator(phase, &extended);
        phase += freq * dt;

        sample = apply_filters(sample, &extended, t, &fs);
        sample *= extended.volume;

        if (sample >  1.0f) sample =  1.0f;
        if (sample < -1.0f) sample = -1.0f;
        out->samples[i] = (int16_t)(sample * 32767.0f);
    }

    /* Apply crossfade at loop boundary for seamless looping */
    int crossfade_samples = (int)(def->crossfade * AUDIO_SAMPLE_RATE);
    if (crossfade_samples > sample_count / 2)
        crossfade_samples = sample_count / 2;

    for (int i = 0; i < crossfade_samples; i++) {
        float blend = (float)i / (float)crossfade_samples;
        int end_idx = sample_count - crossfade_samples + i;
        float s_start = (float)out->samples[i] / 32767.0f;
        float s_end   = (float)out->samples[end_idx] / 32767.0f;

        /* Blend end into start */
        float merged = s_start * blend + s_end * (1.0f - blend);
        if (merged >  1.0f) merged =  1.0f;
        if (merged < -1.0f) merged = -1.0f;

        out->samples[i] = (int16_t)(merged * 32767.0f);
    }
    /* Trim the crossfade tail so the loop point is clean */
    out->sample_count = sample_count - crossfade_samples;
}
