#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <profan/syscall.h>
#include <profan.h>
#include <unistd.h>




#define _pscall(module, id, ...) \
    profan_syscall(((module << 24) | id), __VA_ARGS__)

void hda_play(uint32_t sample_rate, void* pcm_data, uint32_t pcm_octets, uint16_t lvi);
int hda_is_supported_sample_rate(uint32_t sample_rate);
void hda_set_volume(uint32_t volume);
uint32_t hda_get_pos(void);
void hda_check_headphone_connection_change(void);
void hda_stop(void);

#define hda_play(b, c, d, e) ((void) _pscall(7, 0, b, c, d, e))
#define hda_is_supported_sample_rate(b) ((int) _pscall(7, 1, b))
#define hda_set_volume(b) ((void) _pscall(7, 2, b))
#define hda_get_pos() ((uint32_t) _pscall(7, 3, 0))
#define hda_check_headphone_connection_change() ((void) _pscall(7, 4, 0))
#define hda_stop() ((void) _pscall(7, 5, 0))

typedef struct {
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t length;
    uint32_t flags;
} __attribute__((packed)) hda_play_buffer_t;  



typedef struct {
    uint16_t audio_format;   // PCM = 1
    uint16_t num_channels;   // Mono = 1, Stereo = 2...
    uint32_t sample_rate;    // 8000, 44100...
    uint32_t byte_rate;      // sample_rate * num_channels * Bytes Per Sample
    uint16_t block_align;    // num_channels * Bytes Per Sample
    uint16_t bits_per_sample; // 8, 16, etc.

    uint32_t data_bytes;     // Taille des données audio
    uint32_t data_offset;    // Offset vers le chunk "data"

    char *artist;
    char *title;
    char *album;
    char *year;
} wav_file_t;

char *header_set_info(FILE *file, uint32_t size) {
    char *info = malloc(size + 1);
    fread(info, 1, size, file);
    info[size] = 0;
    return info;
}

wav_file_t *read_header(FILE *file) {

    char riff_header[4], wave_header[4];
    uint32_t riff_size;

    if (fread(riff_header, 1, 4, file) != 4)
        return NULL;

    fread(&riff_size, 4, 1, file);
    fread(wave_header, 1, 4, file);

    if (memcmp(riff_header, "RIFF", 4) != 0 || memcmp(wave_header, "WAVE", 4) != 0)
        return NULL;

    wav_file_t *header = calloc(1, sizeof(wav_file_t));

    char chunk_id[4];
    uint32_t chunk_size;

    while (fread(chunk_id, 1, 4, file) == 4 &&
           fread(&chunk_size, 4, 1, file) == 1) {

        if (memcmp(chunk_id, "fmt ", 4) == 0) {
            fread(&header->audio_format,    sizeof(uint16_t), 1, file);
            fread(&header->num_channels,    sizeof(uint16_t), 1, file);
            fread(&header->sample_rate,     sizeof(uint32_t), 1, file);
            fread(&header->byte_rate,       sizeof(uint32_t), 1, file);
            fread(&header->block_align,     sizeof(uint16_t), 1, file);
            fread(&header->bits_per_sample, sizeof(uint16_t), 1, file);

            if (chunk_size > 16)
                fseek(file, chunk_size - 16, SEEK_CUR);
        }
        else if (memcmp(chunk_id, "data", 4) == 0) {
            header->data_bytes = chunk_size;
            header->data_offset = ftell(file);
            break;
        }
        else if (memcmp(chunk_id, "LIST", 4) == 0) {
            long list_end = ftell(file) + chunk_size;

            char list_type[4];
            fread(list_type, 1, 4, file);
            if (memcmp(list_type, "INFO", 4) == 0) {
                while (ftell(file) < list_end) {
                    char info_id[4];
                    uint32_t info_size;
                    if (fread(info_id, 1, 4, file) != 4)
                        break;
                    if (fread(&info_size, 4, 1, file) != 1)
                        break;

                    if (memcmp(info_id, "IART", 4) == 0)
                        header->artist = header_set_info(file, info_size);
                    else if (memcmp(info_id, "INAM", 4) == 0)
                        header->title = header_set_info(file, info_size);
                    else if (memcmp(info_id, "IPRD", 4) == 0)
                        header->album = header_set_info(file, info_size);
                    else if (memcmp(info_id, "ICRD", 4) == 0)
                        header->year = header_set_info(file, info_size);
                    else
                        fseek(file, info_size, SEEK_CUR);

                    if (info_size % 2 == 1)
                        fseek(file, 1, SEEK_CUR);
                }
            } else {
                fseek(file, list_end - ftell(file), SEEK_CUR);
            }
        }
        else {
            fseek(file, chunk_size, SEEK_CUR);
        }
    }

    return header;
}


#define BLOCK_SIZE (64 * 1024)
#define CIRUCULAR_COUNT 4

int add_block_to_buffer(FILE *file, wav_file_t *header, void *at, uint32_t block_index) {
    size_t to_read = BLOCK_SIZE;

    fseek(file, header->data_offset + (block_index * BLOCK_SIZE), SEEK_SET);

    if ((block_index + 1) * BLOCK_SIZE > header->data_bytes)
        to_read = header->data_bytes - (block_index * BLOCK_SIZE);


    size_t read = fread(at, 1, to_read, file);

    if (read < BLOCK_SIZE)
        memset(at + read, 0, BLOCK_SIZE - read);

    int sum = 0;
    for (size_t i = 0; i < read; i++)
        sum += ((uint8_t *)at)[i];

    return read;
}

void print_info(wav_file_t *header, uint32_t pos) {
    int curent_sec = pos / header->byte_rate;
    int total_sec = header->data_bytes / header->byte_rate;

    if (header->artist)
        printf("\r%s - %s", header->artist, header->title);
    else
        printf("\r%s", header->title);
    if (header->year)
        printf(" (%s)", header->year);

    printf(" | %02d:%02d / %02d:%02d",
            curent_sec / 60, curent_sec % 60, total_sec / 60, total_sec % 60
    );

    fflush(stdout);
}

int start_play(FILE *file, wav_file_t *header) {
    if (hda_is_supported_sample_rate(header->sample_rate)) {
        printf("Error: Sample rate %d not supported by HDA\n", header->sample_rate);
        return 1;
    }

    void *physical = syscall_mem_alloc(BLOCK_SIZE * 4, 1, 0x1000);

    hda_play_buffer_t *buf = syscall_mem_alloc(sizeof(hda_play_buffer_t) * CIRUCULAR_COUNT, 1, 0x1000);

    for (int i = 0; i < CIRUCULAR_COUNT; i++) {
        buf[i].addr_low = ((uint32_t) physical) + (i * BLOCK_SIZE);
        buf[i].addr_high = 0;
        buf[i].length = BLOCK_SIZE;
        buf[i].flags = 0;
    }

    int pos, block = 0;

    for (int i = 0; i < CIRUCULAR_COUNT; i++)
        add_block_to_buffer(file, header, physical + i * BLOCK_SIZE, block++);

    hda_play(header->sample_rate, buf, CIRUCULAR_COUNT * BLOCK_SIZE * 2, CIRUCULAR_COUNT - 1);

    int loop_reset = 0;
    int old_pos = 0;

    int end_count = 0;
    
    while (1) {
        while (1) {
            pos = hda_get_pos();

            if (old_pos > pos)
                loop_reset++;

            old_pos = pos;

            if (pos / BLOCK_SIZE + (loop_reset * CIRUCULAR_COUNT) > (block - CIRUCULAR_COUNT))
                break;

            print_info(header, pos + (loop_reset * CIRUCULAR_COUNT * BLOCK_SIZE));

            usleep(10000);
        }

        if (add_block_to_buffer(file, header, physical + ((block % CIRUCULAR_COUNT) * BLOCK_SIZE), block) == 0)
            end_count++;

        if (end_count >= CIRUCULAR_COUNT - 1)
            break;

        asm("wbinvd");
        block++;
    }

    hda_stop();

    print_info(header, header->data_bytes);
    printf("\n");

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <wav file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");

    if (!file) {
        printf("Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    wav_file_t *header = read_header(file);


    if (!header) {
        printf("Error: Not a valid WAV file\n");
        fclose(file);
        return 1;
    }

    if (!header->title)
        header->title = strdup(argv[1]);

    printf("playing %d channels at %d kbps: %d bits %gkHz\n",
            header->num_channels, header->byte_rate / 125,
            header->bits_per_sample, header->sample_rate / 1000.0
    );

    if (header->audio_format != 1) {
        printf("Error: Only PCM format is supported\n");
        goto end;
    }

    if (header->num_channels != 2) {
        printf("Error: Only stereo files are supported\n");
        goto end;
    }

    if (header->bits_per_sample != 16) {
        printf("Error: Only 16 bits files are supported for now\n");
        goto end;
    }

    if (start_play(file, header) != 0) {
        goto end;
    }

    end:

    free(header->artist);
    free(header->title);
    free(header->album);
    free(header->year);

    free(header);
    fclose(file);

    return 0;
}
