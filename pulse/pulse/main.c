/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Hyperloop Pulse Ana Program Gövdesi
  ******************************************************************************
  * @attention
  *
  * Bu dosya STM32CubeMX tarafından üretilen yapı üzerine kurulmuştur.
  * Tüm kontrol lojiği control.c üzerinden yönetilir.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>

// Proje Modüllerini Dahil Et
#include "control.h"       // Ana Kontrol Döngüsü
#include "communication.h" // İletişim ve UART Callback İşlemleri
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
// UART Kesmesi için Global Tampon (1 Baytlık)
uint8_t rx_data; 
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
// printf fonksiyonunun UART'a yönlendirilmesi için gerekli prototip
int _write(int file, char *ptr, int len);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// --- PRINTF YÖNLENDİRMESİ ---
int _write(int file, char *ptr, int len)
{
  // printf çağrıldığında veriyi UART1 üzerinden gönderir
  HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, 100);
  return len;
}

// --- UART ALIM KESMESİ (RX COMPLETE CALLBACK) ---
// Donanım 1 bayt veri aldığında bu fonksiyon otomatik çağrılır.
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // 1. Gelen baytı İletişim Katmanına (communication.c) gönder
        COMM_ProcessByte(rx_data);

        // 2. Bir sonraki baytı beklemek için Kesmeyi tekrar kur
        HAL_UART_Receive_IT(&huart1, &rx_data, 1);
    }
}

// --- I2C (MPU9250) DMA TAMAMLANDI KESMESİ ---
// MPU9250 verisi DMA ile belleğe yazıldığında çağrılır.
// (Not: Bu fonksiyonu mpu9250_driver.c içinde tanımladıysanız burada tekrar tanımlamayın!
// Eğer orada "weak" tanımlı değilse hata verebilir. Genelde HAL callbackleri main.c'de tutulur
// ama modülerlik için driver içine taşıdıysanız burayı yorum satırı yapın.)



// I2C DMA Tamamlandı Callback
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1) { // MPU9250'nin bağlı olduğu I2C
        MPU9250_Read_DMA_Complete_Callback();
    }
}

// I2C Hata Callback
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1) {
        MPU9250_Error_Callback();
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();      // DMA Init (I2C'den önce olmalı!)
  MX_ADC1_Init();     // NTC için
  MX_I2C1_Init();     // MPU9250 için
  MX_TIM3_Init();     // Encoder için
  MX_USART1_UART_Init(); // Haberleşme için
  /* USER CODE BEGIN 2 */

  // --- SİSTEM BAŞLATMA ---
  // Tüm donanımlar (HAL) kurulduktan sonra kendi init fonksiyonumuzu çağırıyoruz.
  // Bu fonksiyon; sensörleri başlatır, varsayılan değerleri atar ve UART dinlemeyi açar.
  
  CONTROL_Init(); 

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // --- ANA DÖNGÜ ---
    // Burada sadece CONTROL_Loop çağrılır. Tüm mantık oradadır.
    
    CONTROL_Loop();
    
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  // ... (CubeMX tarafından otomatik doldurulur) ...
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{
  // ... (CubeMX tarafından otomatik doldurulur) ...
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
  // ... (CubeMX tarafından otomatik doldurulur) ...
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{
  // ... (CubeMX tarafından otomatik doldurulur) ...
  // DİKKAT: Encoder Mode seçili olmalı!
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
  // ... (CubeMX tarafından otomatik doldurulur) ...
}

/**
  * @brief DMA Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA_Init(void)
{
  // ... (CubeMX tarafından otomatik doldurulur) ...
  // DİKKAT: NVIC Settings sekmesinden DMA kesmesini (global interrupt) açmayı unutmayın!
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  // ... (CubeMX tarafından otomatik doldurulur) ...
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  * where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

