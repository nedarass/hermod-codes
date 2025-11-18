#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "encoder_read.pio.h"
#include "../../include/sensors/encoder_read.h"   /* PIN_A, PIN_B, PIN_Z, ENCODER_SM */

static volatile int32_t encoder_position = 0;

/* --- yön çözüm tablosu (prev<<2 | now) → −1 / 0 / +1 --- */
static const int8_t dir_table[16] = {
/* 00→ */  0, +1, -1,  0,
           -1, 0,  0, +1,
           +1, 0,  0, -1,
            0,-1, +1,  0 };

/* ----------  IRQ ---------- */
static void __time_critical_func(pio0_irq_handler)(void)
{
    PIO pio = pio0;
    static uint8_t prev = 0;      /* önceki AB kodu (yalnızca ISR içinde kullanılır) */

    while (!pio_sm_is_rx_fifo_empty(pio, ENCODER_SM)) {
        uint8_t ab = (uint8_t) pio_sm_get(pio, ENCODER_SM) & 0x03;
        int8_t   step = dir_table[(prev << 2) | ab];
        prev = ab;
        encoder_position += step;
    }

    /* Z fazı darbesi (aktif-yüksek) algılanınca sıfırla */
    if (gpio_get(PIN_Z))
        encoder_position = 0;
}

/* Dışarıdan okuma */
int32_t encoder_get_position(void)
{
    return encoder_position;
}

/* ----------  Kurulum ---------- */
int encoder_setup(void)
{
    /* PIO programı yükle */
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &encoder_read_program);
    pio_sm_config cfg = encoder_read_program_get_default_config(offset);

    /* Giriş pinleri */
    pio_gpio_init(pio, PIN_A);
    pio_gpio_init(pio, PIN_B);
    gpio_set_dir(PIN_A, GPIO_IN);
    gpio_set_dir(PIN_B, GPIO_IN);
    pio_sm_set_consecutive_pindirs(pio, ENCODER_SM, PIN_A, 2, false);

    sm_config_set_in_pins  (&cfg, PIN_A);
    sm_config_set_fifo_join(&cfg, PIO_FIFO_JOIN_RX);
    sm_config_set_clkdiv   (&cfg, 1.0f);          /* sysclk /1 → 133 MHz */

    pio_sm_init(pio, ENCODER_SM, offset, &cfg);
    pio_sm_set_enabled(pio, ENCODER_SM, true);

    /* Z fazı (indeks) pini */
    gpio_init(PIN_Z);
    gpio_set_dir(PIN_Z, GPIO_IN);
    gpio_pull_down(PIN_Z);

    /* IRQ – RX FIFO boş-değil olayı SM0/1/2/3’e göre seç */
    irq_set_exclusive_handler(PIO0_IRQ_0, pio0_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);
    #if defined(pis_sm0_rxnempty)
    const enum pio_interrupt_source irq_source =
        (enum pio_interrupt_source)(pis_sm0_rxnempty + ENCODER_SM);
#else
    const enum pio_interrupt_source irq_source =
        (enum pio_interrupt_source)(PIO_INTR_SM0_RXNEMPTY_LSB + ENCODER_SM);
#endif

    pio_set_irq0_source_enabled(pio, irq_source, true);

    return 0;
}
/*// Core/Src/encoder_driver.c

#include "encoder_driver.h"
#include "shared_data.h"

// Global sistem durum yapısı
extern System_State_t g_system_state;

// Sabitler
#define PULSES_PER_REVOLUTION 4096.0f // Encoder tur başına darbe sayısı (Kullanılan donanıma göre değiştirilmeli)
#define WHEEL_CIRCUMFERENCE 0.5f      // Tekerlek çevresi (metre)

// Enkoderden metre/saniye hesaplama için gerekli (Genellikle 10ms'de bir ölçülür)
#define UPDATE_PERIOD_SEC 0.01f

static int32_t last_count = 0; // Son pozisyon sayacı değeri

void ENCODER_Init(TIM_HandleTypeDef *htim)
{
    // STM32CubeMX'te TIM'in Encoder Modunda başlatılmış olması gerekir.
    HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_ALL);
    last_count = __HAL_TIM_GET_COUNTER(htim);
    printf("Encoder Baslatildi.\r\n");
}

void ENCODER_Update(TIM_HandleTypeDef *htim)
{
    int32_t current_count = __HAL_TIM_GET_COUNTER(htim);

    // Counter resetlenmiş olabilir (Örn: 16-bit Timer taşması)
    int32_t count_delta = current_count - last_count;

    // Pozisyon ve Hız Hesaplama
    float distance_delta_m = (float)count_delta / PULSES_PER_REVOLUTION * WHEEL_CIRCUMFERENCE;

    // Hız: (Mesafe / Zaman)
    float current_velocity_mps = distance_delta_m / UPDATE_PERIOD_SEC;

    // Global Durumu Güncelle
    g_system_state.velocity_mps = current_velocity_mps;
    g_system_state.position_m += distance_delta_m;

    last_count = current_count;
}*/
