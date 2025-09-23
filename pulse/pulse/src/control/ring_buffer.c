#include "../../include/control/ring_buffer.h"
#include "pico/critical_section.h"

static critical_section_t rb_critical_section;
static bool critical_section_initialized = false;

void ring_buffer_init(ring_buffer_t *rb) {
    if (!critical_section_initialized) {
        critical_section_init(&rb_critical_section);
        critical_section_initialized = true;
    }
    
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

bool ring_buffer_push(ring_buffer_t *rb, uint8_t data) {
    critical_section_enter_blocking(&rb_critical_section);
    
    if (rb->count >= RING_BUFFER_SIZE) {
        critical_section_exit(&rb_critical_section);
        return false;
    }
    
    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % RING_BUFFER_SIZE;
    rb->count++;
    
    critical_section_exit(&rb_critical_section);
    return true;
}

bool ring_buffer_pop(ring_buffer_t *rb, uint8_t *data) {
    critical_section_enter_blocking(&rb_critical_section);
    
    if (rb->count == 0) {
        critical_section_exit(&rb_critical_section);
        return false;
    }
    
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % RING_BUFFER_SIZE;
    rb->count--;
    
    critical_section_exit(&rb_critical_section);
    return true;
}

bool ring_buffer_is_empty(ring_buffer_t *rb) {
    critical_section_enter_blocking(&rb_critical_section);
    bool empty = (rb->count == 0);
    critical_section_exit(&rb_critical_section);
    return empty;
}

bool ring_buffer_is_full(ring_buffer_t *rb) {
    critical_section_enter_blocking(&rb_critical_section);
    bool full = (rb->count >= RING_BUFFER_SIZE);
    critical_section_exit(&rb_critical_section);
    return full;
}

uint16_t ring_buffer_get_count(ring_buffer_t *rb) {
    critical_section_enter_blocking(&rb_critical_section);
    uint16_t count = rb->count;
    critical_section_exit(&rb_critical_section);
    return count;
}

bool ring_buffer_peek_range(ring_buffer_t *rb, uint8_t *data, uint16_t start, uint16_t length) {
    critical_section_enter_blocking(&rb_critical_section);
    
    if (start + length > rb->count) {
        critical_section_exit(&rb_critical_section);
        return false;
    }
    
    for (uint16_t i = 0; i < length; i++) {
        uint16_t index = (rb->tail + start + i) % RING_BUFFER_SIZE;
        data[i] = rb->buffer[index];
    }
    
    critical_section_exit(&rb_critical_section);
    return true;
}

void ring_buffer_discard(ring_buffer_t *rb, uint16_t count) {
    critical_section_enter_blocking(&rb_critical_section);
    
    if (count > rb->count) {
        count = rb->count;
    }
    
    rb->tail = (rb->tail + count) % RING_BUFFER_SIZE;
    rb->count -= count;
    
    critical_section_exit(&rb_critical_section);
}
