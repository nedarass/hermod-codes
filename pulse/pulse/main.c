/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Hyperloop Pulse Ana Kontrol Programı
  * 
  * @description    : Bu dosya STM32CubeMX tarafından üretilen yapı üzerine kurulmuştur.
  *                   Tüm kontrol lojiği control.c üzerinden yönetilir.
  * 
  * @attention      : CubeMX'te aşağıdaki ayarlar yapılmalıdır:
  *                   - Timer 6: 10ms interrupt (100Hz)
  *                   - TIM3: Encoder Mode (A ve B channel)
  *                   - I2C1: DMA RX aktif (MPU9250 için)
  *                   - USART1: RX interrupt aktif
  *                   - GPIO: Tüm actuator ve sensor pinleri
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "control.h"
#include "communication.h"
#include "shared_data.h"
#include "actuators/power_cut.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
// CubeMX tarafından oluşturulan handle'lar
ADC_HandleTypeDef hadc1;      // NTC sensörü için
I2C_HandleTypeDef hi2c1;      // MPU9250 için
TIM_HandleTypeDef htim3;      // Encoder için
TIM_HandleTypeDef htim6;      // 10ms timer için (KRİTİK!)
UART_HandleTypeDef huart1;    // UART iletişim için

/* USER CODE BEGIN PV */
// Global değişkenler
uint8_t rx_data;              // UART RX interrupt için 1-byte buffer
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM6_Init(void);      // 10ms timer EKLENDİ
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
// printf yönlendirme fonksiyonu
int _write(int file, char *ptr, int len);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// ============================================================================
// 1. PRINTF YÖNLENDİRMESİ (UART1 üzerinden)
// ============================================================================
int _write(int file, char *ptr, int len)
{
    // printf çıktısını UART1'e yönlendir (non-blocking)
    HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, 100);
    return len;
}

// ============================================================================
// 2. UART RX INTERRUPT CALLBACK (Komut alımı)
// ============================================================================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // Sadece USART1'den gelen verileri işle
    if (huart->Instance == USART1)
    {
        // Gelen byte'ı communication modülüne gönder
        COMM_ProcessByte(rx_data);
        
        // Bir sonraki byte'ı dinlemeye devam et
        HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    }
}

// ============================================================================
// 3. TIMER 6 INTERRUPT CALLBACK (10ms - 100Hz) - KRİTİK!
// ============================================================================
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // Timer 6: 10ms periyot ile ana kontrol işlemleri
    if (htim->Instance == TIM6)
    {
        // 10ms'lik periyodik görevler buraya
        // (Opsiyonel: Shared_data güncelleme vs.)
    }
}

// ============================================================================
// 4. I2C DMA CALLBACK (MPU9250 okuma tamamlandığında)
// ============================================================================
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // MPU9250 verisi DMA ile tamamen okundu
    if (hi2c->Instance == I2C1)
    {
        // MPU9250.c'deki callback fonksiyonunu çağır
        // Not: Eğer MPU9250_Read_DMA_Complete_Callback tanımlı değilse,
        // bu satırı yorum satırı yapın
        // MPU9250_Read_DMA_Complete_Callback();
    }
}

// ============================================================================
// 5. EXTI (Dış Kesme) CALLBACK - Optik sensörler için
// ============================================================================
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // Omron/optik sensör interrupt'larını optics.c'ye yönlendir
    // Not: optics.h'da OPTICS_EXTI_Callback fonksiyonu olmalı
    OPTICS_EXTI_Callback(GPIO_Pin);
}

/* USER CODE END 0 */

// ============================================================================
// ANA PROGRAM
// ============================================================================
int main(void)
{
    /* MCU Configuration--------------------------------------------------------*/
    HAL_Init();                         // HAL kütüphanesini başlat
    SystemClock_Config();               // Sistem saatini yapılandır
    
    /* Initialize all configured peripherals */
    MX_GPIO_Init();                     // GPIO'ları başlat
    MX_DMA_Init();                      // DMA (I2C'den önce!)
    MX_ADC1_Init();                     // NTC sensör ADC'si
    MX_I2C1_Init();                     // MPU9250 I2C
    MX_TIM3_Init();                     // Encoder Timer
    MX_TIM6_Init();                     // 10ms Timer (KRİTİK!)
    MX_USART1_UART_Init();              // UART iletişim

    /* USER CODE BEGIN 2 */
    
    printf("\r\n");
    printf("========================================\r\n");
    printf("   HYPERLOOP PULSE - STM32 Kontrol     \r\n");
    printf("   Sistem Baslatiliyor...              \r\n");
    printf("========================================\r\n");
    
    // 1. Kontrol sistemini başlat
    CONTROL_Init();
    
    // 2. UART RX interrupt'ını aktif et
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    
    // 3. Timer 6'yı başlat (10ms interrupt)
    HAL_TIM_Base_Start_IT(&htim6);
    
    // 4. Encoder timer'ı başlat
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    
    printf("Sistem hazir. Ana dongu baslatildi.\r\n");
    
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        // ANA KONTROL DÖNGÜSÜ
        // - Sensor okuma, füzyon, karar verme işlemleri
        // - Telemetri gönderimi
        // - Hata yönetimi
        CONTROL_Loop();
        
        // Küçük bir gecikme (opsiyonel, watchdog için)
        HAL_Delay(1);
    }
    /* USER CODE END 3 */
}

// ============================================================================
// SİSTEM HATA HANDLER'ı
// ============================================================================
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    
    printf("!!! KRİTİK HATA !!! Sistemi güvenli moda aliyorum...\r\n");
    
    // 1. ACİL DURUM FRENLEMESİ
    POWERCUT_TriggerEmergency();
    
    // 2. Tüm interrupt'ları devre dışı bırak
    __disable_irq();
    
    // 3. Sonsuz döngüde bekle (hard reset gerekir)
    while (1)
    {
        // LED ile heartbeat gösterebilirsiniz
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        HAL_Delay(500);
    }
    
    /* USER CODE END Error_Handler_Debug */
}

// ============================================================================
// CUBEMX TARAFINDAN OLUŞTURULAN FONKSİYONLAR
// (Aşağıdaki kısım CubeMX tarafından otomatik doldurulacak)
// ============================================================================

void SystemClock_Config(void) {
    // CubeMX otomatik doldurur
}

static void MX_GPIO_Init(void) {
    // CubeMX otomatik doldurur - Tüm GPIO konfigürasyonları
}

static void MX_DMA_Init(void) {
    // CubeMX otomatik doldurur - DMA kanalları
}

static void MX_ADC1_Init(void) {
    // CubeMX otomatik doldurur - NTC ADC kanalı
}

static void MX_I2C1_Init(void) {
    // CubeMX otomatik doldurur - MPU9250 I2C
}

static void MX_TIM3_Init(void) {
    // CubeMX otomatik doldurur - ENCODER MODE seçili olmalı!
}

static void MX_TIM6_Init(void) {
    // CubeMX otomatik doldurur - 10ms period (100Hz)
    // Prescaler ve Period değerleri saat hızına göre ayarlanmalı
}

static void MX_USART1_UART_Init(void) {
    // CubeMX otomatik doldurur - Baud rate: 115200 önerilir
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
    printf("Assert failed: %s, line %lu\r\n", file, line);
    Error_Handler();
}
#endif
