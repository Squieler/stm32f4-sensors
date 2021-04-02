# STM32F4 (Tested on STM32F411RE) - ADS1115 STM32 Single-Ended, Single-Shot, PGA & Data Rate Enabled ADC Library
A simple C library (STM32 HAL) for single-ended single-shot ADC measurments with ADS1115 module.

This library includes several configurations such as PGA and data rate of ADS1115 and tested on STM32F411RE. Example project available.

Don't forget to check datasheet of ADS111x for detailed information: https://www.ti.com/lit/gpn/ADS1113

# [Türkçe/Turkish] Kullanım Talimatı

 * 1) Adress değişkenini güncelleyin:
 	ADDR PIN --> GND ise 1001000
 	ADDR PIN --> VDD ise 1001001
 	ADDR PIN --> SDA ise 1001010
 	ADDR PIN --> SCL ise 1001000

 * 2) ADS1115_Init(...) fonks. ile I2C peripheral ve PGA ile Data Rate ayarlarını yapın. (HAL_OK veya HAL_ERROR)
 * 3) ADS1115_readSingleEnded(...) fonksiyonu ile single-shot okuma yapacağınız portu seçin ve float tipinde değişkeninizin adresini gönderin.
 * 4) Üçüncü adımdan sonra değişkeninizin içerisinde uygun katsayıyla çarpılmış gerilim değeri saklanacaktır.