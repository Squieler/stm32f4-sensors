/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
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
#include <stdlib.h>
#include <stdarg.h> //for va_list var arg functions
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SIZE 300 // Buffer boyutu.
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */
uint8_t flag = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	/* Eğer huart1 buffer dolduysa, bayrağı işaretle. */
	flag = 1;

}

float GPS_nmea_to_dec(float deg_coord, char nsew) {
    int degree = (int)(deg_coord/100);
    float minutes = deg_coord - degree*100;
    float dec_deg = minutes / 60;
    float decimal = degree + dec_deg;
    if (nsew == 'S' || nsew == 'W') { // return negative
        decimal *= -1;
    }
    return decimal;
}
int nmea0183_checksum(char *msg) {


	int checksum = 0;
	int j = 0;

	// the first $ sign and the last two bytes of original CRC + the * sign
	for (j = 1; j < strlen(msg) - 4; j++) {
		checksum = checksum ^ (unsigned) msg[j];
	}

	return checksum;
}

void serialPrint(const char *fmt, ...) {
	static char buffer[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), -1); // UART defined at huart2 for serial communication with computer.

}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

	/* Recieve buffer */
	uint8_t gpsRx[SIZE];
	memset(gpsRx, 0, SIZE); // Bufferı temizle.

	/* DMA'yı huart1 için başlat; circular mode */
	HAL_UART_Receive_DMA(&huart1, gpsRx, SIZE);

	/* Gerekli değişkenler */
	char stringBuffer[SIZE + 1]; // stringBuffer --> Integer gelen uart verisini, char'a çevirip saklamak için. (+1 eklentisi, en sona NULL eklemek için.)
	memset(stringBuffer, 0, SIZE + 1); // Buffer gereksiz veriden temizleniyor.

	char nmeaStr[100]; // NMEA verisini saklayacağımız değişken.
	memset(nmeaStr, 0, 100); // Buffer temizleniyor.

	/* Gelen bilgiyi parçalara bölmek için kullanılacak fonksiyonlar için değişkenler. (strsep ve strdup) */
	char *token, *string, *stringFree;
	char *rawSum;	// NMEA checksum değişkeni.
	char smNmbr[3]; // NMEA checksum değişkeni.
	uint8_t intSum;
	char hex[2]; // Checksum saklama değişkeni.

	//GPS bilgisi için veri yapısı.
	typedef struct{

	    // calculated values
		float dec_longitude;
	    float dec_latitude;

	    // GGA - Global Positioning System Fixed Data
	    float nmea_longitude;
	    float nmea_latitude;
	    float utc_time;
	    char ns, ew;

	    // GPS - RMC
	    float speed_k;
	    int date;

	    // GLL
	    char gll_status;

	} GPS_t;
	GPS_t gpsData;
	gpsData.date = 00000000;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

		/* Eğer buffer dolduysa devam et. */
		if (flag == 1) {

			/* Gelen veriyi bilgisayara yolla. */
			//HAL_UART_Transmit(&huart2, gpsRx, SIZE, 200); // Debug maksatlı, istenilirse kapatılabilir.

			/* Integer buffer ---> string buffer */
			sprintf(stringBuffer, "%s", gpsRx);

			/* Gelen veri, char'a çeviliyor. (Integer'den) */
			string = strdup(stringBuffer); // stringFree ile işimiz bitince belleği geri bırakacağız.

			stringFree = string;

			/* Gelen NMEA verisini, satır sonuna göre bölünüyor. (\n) */
			while( (token = strsep(&string, "\n")) != NULL ){

				memset(nmeaStr, 0, 100); // Eski verileri siliyoruz.
				sprintf(nmeaStr, "%s", token); // Token verisi, işlenmek üzere char array'e aktarılıyor.

				/* Cümlelere bölünen verileri, istediğimize göre filtreleyeceğiz. */
				/* İyi bir NMEA cümlesi $GPRMC ile başlar, * ile biter ve en az 49 karakter uzunluğunda olur. */
				if ((strstr(nmeaStr, "$GPRMC") != 0) && strlen(nmeaStr) > 49 && strstr(nmeaStr, "*") != 0) {

					/* NMEA checksum */
					rawSum = strstr(nmeaStr, "*"); // * ile başlyan checksum verisi depolanır.
					memcpy(smNmbr, &rawSum[1], 2); // *'dan sonrası smNmbr değişkenine kopyalanır.
					smNmbr[2] = '\0'; // smNmbr değişkenin sonuna satır sonu işaretçisi eklenir.
					intSum = nmea0183_checksum(nmeaStr); // Checksum hesabı yapıyoruz.
					sprintf(hex, "%X", intSum); // Kıyaslayabilmek için checksum'u hex formatında kaydediyoruz.

					/* Checksum kontrolü. */
					if (strstr(smNmbr, hex) != NULL) {

						/* Gelen veriyi bilgisayara aktarmak için... */
						//HAL_UART_Transmit(&huart2, (uint8_t*) nmeaStr, 100, 70);
						//HAL_UART_Transmit(&huart2, (uint8_t*) "\n", 1, 200);

						/* Chekcsum doğru, gelen NMEA cümlesinde hata yok. Cümleyi parçalara bölebiliriz. */
						sscanf(nmeaStr, "$GPRMC,%f,%c,%f,%c,%f,%c,%f,,%d", &gpsData.utc_time, &gpsData.gll_status, &gpsData.nmea_latitude, &gpsData.ns, &gpsData.nmea_longitude, &gpsData.ew, &gpsData.speed_k, &gpsData.date);
						/* Enlem ve boylam bilgisini, yerkürenin bulunduğumuz tarafına göre anlamlandırıp standart formata getiriyoruz. */
						gpsData.dec_latitude = GPS_nmea_to_dec(gpsData.nmea_latitude, gpsData.ns);
						gpsData.dec_longitude = GPS_nmea_to_dec(gpsData.nmea_longitude, gpsData.ew);

						/* Gelen, anlamlandırılmış veriyi bilgisayara anlatmak için. */
						serialPrint("Lat.: %f, Lon.: %f, Speed (knot): %f, UTC: %f, Date: %d\r\n", gpsData.dec_latitude, gpsData.dec_longitude, gpsData.speed_k, gpsData.utc_time, gpsData.date);

					} else if((strstr(nmeaStr, "$GNGLL") != 0) && strlen(nmeaStr) > 49 && strstr(nmeaStr, "*") != 0) {

						/* NMEA checksum */
						rawSum = strstr(nmeaStr, "*"); // * ile başlyan checksum verisi depolanır.
						memcpy(smNmbr, &rawSum[1], 2); // *'dan sonrası smNmbr değişkenine kopyalanır.
						smNmbr[2] = '\0'; // smNmbr değişkenin sonuna satır sonu işaretçisi eklenir.
						intSum = nmea0183_checksum(nmeaStr); // Checksum hesabı yapıyoruz.
						sprintf(hex, "%X", intSum); // Kıyaslayabilmek için checksum'u hex formatında kaydediyoruz.

						/* Checksum kontrolü. */
						if (strstr(smNmbr, hex) != NULL) {

							/* Gelen veriyi bilgisayara aktarmak için... */
							//HAL_UART_Transmit(&huart2, (uint8_t*) nmeaStr, 100, 70);
							//HAL_UART_Transmit(&huart2, (uint8_t*) "\n", 1, 200);

							/* Chekcsum doğru, gelen NMEA cümlesinde hata yok. Cümleyi parçalara bölebiliriz. */
							sscanf(nmeaStr, "$GPGLL,%f,%c,%f,%c,%f,%c", &gpsData.nmea_latitude, &gpsData.ns, &gpsData.nmea_longitude, &gpsData.ew, &gpsData.utc_time, &gpsData.gll_status);
							/* Enlem ve boylam bilgisini, yerkürenin bulunduğumuz tarafına göre anlamlandırıp standart formata getiriyoruz. */
							gpsData.dec_latitude = GPS_nmea_to_dec(gpsData.nmea_latitude, gpsData.ns);
							gpsData.dec_longitude = GPS_nmea_to_dec(gpsData.nmea_longitude, gpsData.ew);

							/* Gelen, anlamlandırılmış veriyi bilgisayara anlatmak için. */
							serialPrint("Lat.: %f, Lon.: %f, UTC: %f, Date: %d\r\n", gpsData.dec_latitude, gpsData.dec_longitude, gpsData.utc_time, gpsData.date);

						}

					}

				} // NMEA cümlesi seçimi bitti.



			} // "\n" ile gelen veriyi cümlelere ayırma bitti.

			/* Bellekte gerekisz yer kaplayan değişkeni temizliyoruz. (Eğer aşağısı yorum olarak kayıt edildiyse, sebebi aşağıdaki fonksiyonların işlemcide kritik hataya sebep olmasıdır.) */
			free(token);
			free(stringFree);

			/* Yeni veri almaya hazırız. */
			flag = 0;
			memset(gpsRx, 0, SIZE);
			HAL_UART_Receive_DMA(&huart1, gpsRx, SIZE);

		}

		/* Yarım saniye gecikme. */
		//HAL_Delay(500);

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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

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
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
