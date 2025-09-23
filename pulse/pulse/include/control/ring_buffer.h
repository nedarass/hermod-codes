#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE 256

typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint16_t count;
} ring_buffer_t;

void ring_buffer_init(ring_buffer_t *rb);
bool ring_buffer_push(ring_buffer_t *rb, uint8_t data);
bool ring_buffer_pop(ring_buffer_t *rb, uint8_t *data);
bool ring_buffer_is_empty(ring_buffer_t *rb);
bool ring_buffer_is_full(ring_buffer_t *rb);
uint16_t ring_buffer_get_count(ring_buffer_t *rb);
bool ring_buffer_peek_range(ring_buffer_t *rb, uint8_t *data, uint16_t start, uint16_t length);
void ring_buffer_discard(ring_buffer_t *rb, uint16_t count);

#endif
