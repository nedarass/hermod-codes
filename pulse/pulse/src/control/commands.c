/*#include "../../include/control/commands.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

// LED is on GPIO 25 (onboard LED)
#define LED_PIN 25

static bool led_initialized = false;

static void init_led(void) {
    if (!led_initialized) {
        gpio_init(LED_PIN);
        gpio_set_dir(LED_PIN, GPIO_OUT);
        led_initialized = true;
    }
}

bool process_command(uint8_t *data) {
    command_t *cmd = (command_t *)data;
    
    // Simple command validation (you can add checksum here)
    switch (cmd->command_id) {
        case CMD_OPEN_LED:
            execute_open_led();
            return true;
            
        case CMD_CLOSE_LED:
            execute_close_led();
            return true;
            
        default:
            // Unknown command
            return false;
    }
}

void execute_open_led(void) {
    init_led();
    gpio_put(LED_PIN, 1);
    printf("LED: ON\n");
}

void execute_close_led(void) {
    init_led();
    gpio_put(LED_PIN, 0);
    printf("LED: OFF\n");
}
*/
// Core/Src/commands.c

#include "commands.h"
#include "shared_data.h"
#include <string.h>
#include <stdio.h>

// Harici değişkenler (main.c'de tanimlanan)
extern UART_HandleTypeDef huart1;
// Global sistem durum yapısı
extern System_State_t g_system_state;

// -------------------------------------------------------------------
// --- SİMULASYON AKTÜATÖR FONKSİYONLARI ---
// -------------------------------------------------------------------

void BRAKE_SetForce(uint8_t force)
{
    // Gerçekte: Fren solenoidi/motoru kontrolü (PWM, Dijital pin vb.)
    g_system_state.brake_status = (force > 10) ? 1 : 0;
    printf("KOMUT: Fren Gucu Ayarlandi: %u, Durum: %s\r\n", force, g_system_state.brake_status ? "AKTIF" : "DEAKTIF");
}

void MOTOR_SetTargetSpeed(float speed_mps)
{
    // Gerçekte: VESC veya Inverter sürücüsüne haberleşme
    printf("KOMUT: Hedef Hizi Ayarlandi: %.2f m/s\r\n", speed_mps);
}

void SYSTEM_EmergencyPowerCut(void)
{
    // Gerçekte: Power Cut rölesi/anahtarı tetiklenir
    printf("KRITIK KOMUT: ACIL GUC KESME! Sistemi kapatiliyor...\r\n");
    g_system_state.error_flags |= ERR_FLAG_POWER_TRIP;
    g_system_state.power_line_status = 0;
}

// -------------------------------------------------------------------
// --- ANA KOMUT İŞLEYİCİ (COMMUNICATION.C'DEN ÇAĞRILIR) ---
// -------------------------------------------------------------------

void COMMAND_ProcessIncoming(uint8_t id, uint8_t type, uint8_t *payload, uint16_t len)
{
    switch (id)
    {
        case CMD_BRAKE_ACTUATE:
        {
            if (type == TYPE_U8 && len == 1) {
                uint8_t brake_force = payload[0];
                BRAKE_SetForce(brake_force);
            } else {
                 printf("HATA: CMD_BRAKE_ACTUATE paketi hatali.\r\n");
            }
            break;
        }
        case CMD_SET_TARGET_SPEED:
        {
            if (type == TYPE_F32 && len == sizeof(float)) {
                float target_speed;
                // C++ std::memcpy yerine C standart memcpy kullanılır
                memcpy(&target_speed, payload, sizeof(float));
                MOTOR_SetTargetSpeed(target_speed);
            } else {
                printf("HATA: CMD_SET_TARGET_SPEED paketi hatali.\r\n");
            }
            break;
        }
        case CMD_POWER_CUT_OFF:
        {
            SYSTEM_EmergencyPowerCut();
            break;
        }
        case CMD_PING_REQUEST:
        {
            // Ping isteği iletişim katmanında (communication.c) cevaplanır.
            printf("PING ISTEDI.\r\n");
            break;
        }
        default:
            printf("BILINMEYEN KOMUT ID: 0x%02X\r\n", id);
            break;
    }
}
