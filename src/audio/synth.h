#ifndef TD_SYNTH_H
#define TD_SYNTH_H

#include "audio.h"

/* ── Waveform types ────────────────────────────────────────────── */

typedef enum WaveType {
    WAVE_SINE = 0,
    WAVE_SQUARE,
    WAVE_SAW,
    WAVE_NOISE,
    WAVE_TRIANGLE,
} WaveType;

/* ── Sound definition (synthesis parameters) ───────────────────── */

typedef struct SoundDef {
    /* Oscillator */
    float base_freq;        /* Hz, 0 = noise-only */
    float freq_slide;       /* Hz/sec change over duration */
    float freq_vibrato;     /* vibrato rate in Hz */
    float vibrato_depth;    /* vibrato depth in Hz */

    /* Envelope (attack → sustain → decay) */
    float attack;           /* seconds */
    float sustain;          /* seconds */
    float decay;            /* seconds */

    /* Waveform mix (0.0 to 1.0 each, mixed together) */
    float wave_sine;
    float wave_square;
    float wave_saw;
    float wave_noise;
    float wave_triangle;

    /* Filter */
    float lpf_cutoff;       /* Hz, 0 = disabled */
    float lpf_sweep;        /* Hz/sec change */
    float hpf_cutoff;       /* Hz, 0 = disabled */
    float hpf_sweep;        /* Hz/sec change */

    /* Output */
    float volume;           /* 0.0 to 1.0 */
} SoundDef;

/* ── Ambient loop definition ───────────────────────────────────── */

typedef struct AmbientDef {
    SoundDef base;          /* base synthesis parameters */
    float    loop_seconds;  /* length of the loop buffer */
    float    crossfade;     /* crossfade region in seconds at loop boundary */
} AmbientDef;

/* ── Synthesis functions ───────────────────────────────────────── */

/* Generate a one-shot PCM buffer from a SoundDef */
void synth_generate(const SoundDef *def, SoundBuffer *out);

/* Generate a seamless looping PCM buffer from an AmbientDef */
void synth_generate_loop(const AmbientDef *def, SoundBuffer *out);

#endif /* TD_SYNTH_H */
