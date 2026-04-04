#include "SPI_&_I2C.h"

#include <Arduino.h>
#include "constants.h"
#include <SPI.h>
#include <Wire.h>

SPIClass vspi(VSPI);
SPIClass hspi(HSPI);

void InitSPI() {
    // Initialize VSPI
    vspi.begin(MPU_SCK, MPU_MISO, MPU_MOSI,-1);

    pinMode(MPU_CS, OUTPUT);
    pinMode(SD_CS, OUTPUT);

    digitalWrite(MPU_CS, HIGH);
    digitalWrite(SD_CS, HIGH);

    // Initialize HSPI
    hspi.begin(FLASH_SCK, FLASH_MISO, FLASH_MOSI, -1);

    pinMode(BME_CS, OUTPUT);
    pinMode(FLASH_CS, OUTPUT);

    digitalWrite(BME_CS, HIGH);
    digitalWrite(FLASH_CS, HIGH);

    /*
    FALTA DEFINIR LA FRECUENCIA Y EL IF PARA VER SI SE INICIALIZA CORRECTAMENTE
    NO SE BIEN QUE ES PERO HAY UNOS MODOS DE SPI QUE SE PUEDEN CONFIGURAR, HABRIA QUE VER CUAL ES EL MAS ADECUADO
    */
}