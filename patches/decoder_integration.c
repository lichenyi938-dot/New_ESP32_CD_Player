// patches/decoder_integration.c
// Example: reading from file system into decoder and sending PCM to I2S
#include <stdint.h>
#include "i2s.h"
#include "decoder_mp3.h" // hypothetical wrapper for Helix

void audio_play_file(const char *path) {
    // 1) open file from ISO9660 wrapper
    // 2) stream data into decoder buffer
    // 3) get PCM frames and write to I2S
}
