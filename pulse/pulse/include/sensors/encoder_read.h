#ifndef ENCODER_READ_H
#define ENCODER_READ_H

#include <stdint.h>

/* --- donanıma göre ayarla --- */
#define PIN_A       2       /* Kanal-A girişi */
#define PIN_B       3       /* Kanal-B girişi */
#define PIN_Z       4       /* İndeks (Z) girişi, opsiyonel */
#define ENCODER_SM  0       /* Kullanılan state machine numarası */

/* ---------- API ---------- */
#ifdef __cplusplus
extern "C" {
#endif

/* PIO-programı yükler, GPIO ve IRQ kurar. 0 döndürür. */
int      encoder_setup(void);

/* Güncel pozisyonu (pulse sayacı) döndürür. */
int32_t  encoder_get_position(void);

#ifdef __cplusplus
}
#endif

#endif /* ENCODER_READ_H */
