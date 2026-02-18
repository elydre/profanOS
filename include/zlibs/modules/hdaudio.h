/*****************************************************************************\
|   === hdaudio.h : 2025 ===                                                  |
|                                                                             |
|    Intel High Definition Audio (HDA) driver header               .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/


#ifndef HDAUDIO_ID
#define HDAUDIO_ID 7

#include <stdint.h>

void     hda_map_memory(void);
uint8_t  hda_is_headphone_connected(void);
void     hda_set_volume(uint32_t volume);
void     hda_check_headphone_connection_change(void);

uint8_t  hda_is_supported_channel_size(uint8_t size);
uint8_t  hda_is_supported_sample_rate(uint32_t sample_rate);
void     hda_play_pcm_data(uint32_t sample_rate, void* pcm_data, uint32_t buffer_size, uint16_t lvi);
void     hda_stop_sound(void);
uint32_t hda_get_stream_position(void);

#ifndef _KERNEL_MODULE

extern int profan_syscall(uint32_t id, ...);

#undef  _pscall
#define _pscall(module, id, ...) \
    profan_syscall(((module << 24) | id), __VA_ARGS__)

#define hda_map_memory()                ((void) _pscall(HDAUDIO_ID, 0, 0))
#define hda_is_headphone_connected()    ((uint8_t) _pscall(HDAUDIO_ID, 1, 0))
#define hda_set_volume(b)               ((void) _pscall(HDAUDIO_ID, 2, b))
#define hda_check_headphone_connection_change() ((void) _pscall(HDAUDIO_ID, 3, 0))
#define hda_is_supported_channel_size(b) ((uint8_t) _pscall(HDAUDIO_ID, 4, b))
#define hda_is_supported_sample_rate(b) ((uint8_t) _pscall(HDAUDIO_ID, 5, b))
#define hda_play_pcm_data(b, c, d, e)   ((void) _pscall(HDAUDIO_ID, 6, b, c, d, e))
#define hda_stop_sound()                ((void) _pscall(HDAUDIO_ID, 7, 0))
#define hda_get_stream_position()       ((uint32_t) _pscall(HDAUDIO_ID, 8, 0))

#endif // _KERNEL_MODULE

#endif
