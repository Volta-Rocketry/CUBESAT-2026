// ──────────── HEADER ────────────

#pragma once

// ──────────── LIBRERIAS ────────────

#include <stdint.h>
#include <stdbool.h>
#include <SPI.h>

// ──────────── CARACTERISTICAS MÓDULO FLASH ────────────

extern SPIClass hspi;

#define FLASH_PAGE_SIZE 256UL
#define FLASH_TOTAL_BYTES 4194304UL
#define PAD_PRESSURE_HPA  1013.25f

// ──────────── PINES ────────────

#define PIN_SCK  18
#define PIN_MISO 19
#define PIN_MOSI 23
#define PIN_CS_FLASH 26
#define PIN_CS_BME 5

// ──────────── BME ────────────

struct StructBME280 {
    uint32_t timestamp;  // Timestamp
    float temp;          // Temperature
    float humidity;      // Humidity
    float pressure;      // Pressure
    float altitude;      // Altitude
};

extern StructBME280 bmeData;

// ──────────── FUNCIONES GLOBALES ────────────

bool flash_init();
void flash_erase_chip();
void flash_write(uint32_t addr, const uint8_t* data, uint16_t len);
void flash_read(uint32_t addr, uint8_t* buf, uint32_t len);
void InitBME280();
void ReadBME280();