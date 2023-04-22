#include <syscall.h>
#include <stdlib.h>

typedef struct {
    uint8_t *content;
    uint32_t size;
    uint32_t write_pos;
    uint32_t current_len;

    uint32_t last_update;
    int id;
} ocm_buffer_t;

ocm_buffer_t **out_buffers;

#define OCM_BUFFER_SIZE 1000
#define OCM_BUFFER_COUNT 10

int main() {
    out_buffers = malloc(sizeof(ocm_buffer_t *) * OCM_BUFFER_COUNT);
    for (int i = 0; i < OCM_BUFFER_COUNT; i++) {
        out_buffers[i] = malloc(sizeof(ocm_buffer_t));
        out_buffers[i]->id = i;
        out_buffers[i]->content = calloc(OCM_BUFFER_SIZE, sizeof(uint8_t));
        out_buffers[i]->size = OCM_BUFFER_SIZE;
        out_buffers[i]->last_update = 0;
        out_buffers[i]->current_len = 0;
        out_buffers[i]->write_pos = 0;
    }
    return 0;
}

uint32_t ocm_write(int id, uint8_t data) {
    ocm_buffer_t *buffer = out_buffers[id];

    if (buffer->current_len < buffer->size) {
        buffer->current_len++;
    }

    buffer->content[buffer->write_pos % buffer->size] = data;
    buffer->last_update = c_timer_get_ms();

    return buffer->write_pos++;
}

uint8_t ocm_read(int id, uint32_t pos) {
    ocm_buffer_t *buffer = out_buffers[id];

    return buffer->content[pos % buffer->size];
}

uint32_t ocm_get_len(int id) {
    ocm_buffer_t *buffer = out_buffers[id];

    return buffer->current_len;
}

uint32_t ocm_get_last_update(int id) {
    ocm_buffer_t *buffer = out_buffers[id];

    return buffer->last_update;
}

uint32_t ocm_get_wpos(int id) {
    ocm_buffer_t *buffer = out_buffers[id];

    return buffer->write_pos;
}

void ocm_set_wpos(int id, uint32_t pos) {
    ocm_buffer_t *buffer = out_buffers[id];

    buffer->write_pos = pos;
}
