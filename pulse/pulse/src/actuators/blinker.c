
// pulse/pulse/src/actuators/blinker.c
#include "blinker.h" // Include yolunu düzelttim
#include "shared_data.h"

/* * DİKKAT: Bu kodun çalışması için CubeMX'te (IOC dosyası) ilgili LED pinine 
 * "BLINKER" etiketini (User Label) vermeniz gerekmektedir!
 *
 * Bu işlem main.h dosyasında BLINKER_GPIO_Port ve BLINKER_Pin 
 * tanımlarını otomatik oluşturur.
 */

static uint32_t last_toggle_time = 0;
static const uint32_t BLINK_PERIOD_MS = 500;
static bool blinking_enabled = false;

// --- BLINKER_Init --- Sistemin başlangıcında LED'i güvenli duruma (kapalı) getirir. Zaman sayacını başlatır.
void BLINKER_Init(void)
{
    BLINKER_Set(false);
    blinking_enabled = false;
    last_toggle_time = HAL_GetTick();
}

// --- BLINKER_Set --- LED'i doğrudan AÇAR (true) veya KAPATIR (false).
// Eğer yanıp sönme modu açıksa onu iptal eder. Durumu shared_data'ya bildirir.
void BLINKER_Set(bool status)
{
    // CubeMX'ten gelen tanımları kullanıyoruz
    HAL_GPIO_WritePin(BLINKER_GPIO_Port, BLINKER_Pin, 
                      status ? GPIO_PIN_SET : GPIO_PIN_RESET);
                      
    blinking_enabled = false;
    
    // Ortak veri havuzuna durumu yaz (Telemetri için)
    shared_data.actuators.blinker_state = status ? BLINKER_STATE_ON : BLINKER_STATE_OFF;
}

// --- BLINKER_Toggle --- Anlık durum değişimi (Manuel tetikleme için)
void BLINKER_Toggle(void)
{
    HAL_GPIO_TogglePin(BLINKER_GPIO_Port, BLINKER_Pin);
}

// --- BLINKER_StartBlinking --- LED'in otomatik yanıp sönme modunu aktif eder.
void BLINKER_StartBlinking(void)
{
    blinking_enabled = true;
    last_toggle_time = HAL_GetTick();
    shared_data.actuators.blinker_state = BLINKER_STATE_BLINKING;
}

// --- BLINKER_StopBlinking --- Yanıp sönme modunu durdurur ve LED'i kapatır.
void BLINKER_StopBlinking(void)
{
    blinking_enabled = false;
    BLINKER_Set(false);
}

// --- BLINKER_UpdateNonBlocking --- while(1) döngüsü içinde sürekli çağrılır. İşlemciyi durdurmadan (delay kullanmadan) zamanı kontrol eder.
// Eğer 500ms geçtiyse LED'in durumunu değiştirir (Toggle).
void BLINKER_UpdateNonBlocking(void)
{
    if (!blinking_enabled) return;
    
    uint32_t current_time = HAL_GetTick();
    
    // Zaman farkı hesaplama (Overflow korumalı)
    if ((current_time - last_toggle_time) >= BLINK_PERIOD_MS)
    {
        HAL_GPIO_TogglePin(BLINKER_GPIO_Port, BLINKER_Pin);
        last_toggle_time = current_time;
    }
}
