#include "flash_storage.h"
#include "comm_manager.h"
#include "constants.h"
#include "error_warning.h"
#include "signals.h"
#include <Arduino.h>
#include <SPI.h>

SPIClass hspi(HSPI);

static void flashWaitBusy() {
    digitalWrite(FLASH_CS, LOW);
    hspi.transfer(CMD_READ_STATUS);
    while (hspi.transfer(0x00) & 0x01);
    digitalWrite(FLASH_CS, HIGH);
}

static void flashWriteEnable() {
    digitalWrite(FLASH_CS, LOW);
    hspi.transfer(CMD_WRITE_ENABLE);
    digitalWrite(FLASH_CS, HIGH);
    delayMicroseconds(5);
}

static void flashWritePage(uint32_t addr, const uint8_t* data, uint16_t len) {
    flashWriteEnable();

    digitalWrite(FLASH_CS, LOW);
    hspi.transfer(CMD_PAGE_PROGRAM); 

    hspi.transfer((addr >> 16) & 0xFF); 
    hspi.transfer((addr >> 8) & 0xFF);   
    hspi.transfer( addr & 0xFF);

    for (uint16_t i = 0; i < len; i++) {
        hspi.transfer(data[i]);
    }

    digitalWrite(FLASH_CS, HIGH);
    flashWaitBusy();
}

void flashInit() {
    pinMode(FLASH_CS, OUTPUT);
    digitalWrite(FLASH_CS, HIGH);
    delay(10);
    
    digitalWrite(FLASH_CS, LOW);
    hspi.transfer(CMD_JEDEC_ID);
    uint8_t manufacturer = hspi.transfer(0x00);
    uint8_t memType     = hspi.transfer(0x00);
    uint8_t capacity     = hspi.transfer(0x00);

    digitalWrite(FLASH_CS, HIGH);

    if (manufacturer == 0xEF && memType == 0x40 && capacity == 0x16) {
        println("Initialized FLASH");
        initSensor.initFlash = 1;
    }
    else {
        criticalErrorSensor("FLASH not found");
        initSensor.initFlash = 0;
    }
}

void flashEraseChip() {
    flashWriteEnable();
    digitalWrite(FLASH_CS, LOW);
    hspi.transfer(CMD_CHIP_ERASE);
    digitalWrite(FLASH_CS, HIGH);
    flashWaitBusy();
}

void flashWrite(uint32_t addr, const uint8_t* data, uint16_t len) {
    uint16_t written = 0;
    uint32_t currentAddr = addr;
    const uint8_t* currentData = data;

    while (written < len) {
        uint16_t pageOffset   = currentAddr % FLASH_PAGE_SIZE;
        uint16_t spaceInPage = FLASH_PAGE_SIZE - pageOffset;

        uint16_t toWrite = (len - written) < spaceInPage ? (len - written) : spaceInPage;
        flashWritePage(currentAddr, currentData, toWrite);

        written += toWrite;
        currentAddr += toWrite;
        currentData += toWrite;
    }
}

void flashRead(uint32_t addr, uint8_t* buf, uint32_t len) {
    digitalWrite(FLASH_CS, LOW);
    hspi.transfer(CMD_READ_DATA);

    hspi.transfer((addr >> 16) & 0xFF);
    hspi.transfer((addr >> 8)  & 0xFF);
    hspi.transfer( addr & 0xFF);

    for (uint32_t i = 0; i < len; i++) {
        buf[i] = hspi.transfer(0x00);
    }

    digitalWrite(FLASH_CS, HIGH);
}

/**
 * @brief Verifies and prints the content in flash.
 * * Reads all the Flash memory where the data was stored, 
 * identifies what type of packet each one is, and verifies if the information is complete
 */
void verifyFlashContent() {
    println("Starting FLASH Verification");
    
    if (gFlashWriteAddr == 0) {
        println("FLASH is empty");
        return;
    }

    Serial.printf("Total written data: %lu bytes\n", gFlashWriteAddr);

    uint32_t currentAddr = 0;
    uint32_t countFast = 0;
    uint32_t countSlow = 0;
    uint32_t countCorrupt = 0;

    Serial.println("TYPE,Packet_TS,"
                "MPU_TS,MPU_ax,MPU_ay,MPU_az,MPU_gx,MPU_gy,MPU_gz,MPU_temp,"
                "QMC_TS,QMC_mx,QMC_my,QMC_mz,"
                "Madg_q0,Madg_q1,Madg_q2,Madg_q3,Madg_beta,"
                "BMP_TS,BMP_temp,BMP_pres,BMP_alt,"
                "BNO_TS,BNO_ax,BNO_ay,BNO_az,BNO_gx,BNO_gy,BNO_gz,"
                "BNO_mx,BNO_my,BNO_mz,"
                "BNO_qw,BNO_qx,BNO_qy,BNO_qz,"
                "BNO_gax,BNO_gay,BNO_gaz,"
                "Filt_Alt,Filt_Vel,Filt_Acc,Filt_Alpha,CRC_Status");
    
    Serial.println("TYPE,Timestamp_ms,"
                "BME_temp,BME_hum,BME_pres,BME_alt,"
                "BMP_temp,BMP_pres,BMP_alt,"
                "GPS_year,GPS_month,GPS_day,GPS_hour,GPS_min,GPS_sec,"
                "GPS_lat,GPS_lon,GPS_alt,GPS_speed,GPS_course,"
                "GPS_sats,GPS_hdop,GPS_valid,CRC_Status");

    while (currentAddr < gFlashWriteAddr) {
        uint8_t id = 0;

        flashRead(currentAddr, &id, 1);

        if (id == 0x01) {
            FastFlightPacket p;
            flashRead(currentAddr, (uint8_t*)&p, sizeof(FastFlightPacket));
            
            uint16_t crc_calc = crc16CCITT((uint8_t*)&p, sizeof(FastFlightPacket) - 2);
            bool ok = (p.checksum == crc_calc);

            Serial.printf("FAST,%lu,"
                          "%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f," // MPU
                          "%lu,%.2f,%.2f,%.2f,"                     // QMC
                          "%.4f,%.4f,%.4f,%.4f,%.4f,"               // Madgwick
                          "%lu,%.2f,%.2f,%.2f,"                     // BMP
                          "%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.4f,%.4f,%.4f,%.4f,%.2f,%.2f,%.2f," // BNO
                          "%.2f,%.2f,%.2f,%.3f,%d\n",               // Filter + CRC
                          p.timestamp_ms,
                          // MPU6050
                          p.mpu.timestamp, p.mpu.MPU_ax, p.mpu.MPU_ay, p.mpu.MPU_az, p.mpu.MPU_gx, p.mpu.MPU_gy, p.mpu.MPU_gz, p.mpu.MPU_temp,
                          // QMC5883L
                          p.qmc.timestamp, p.qmc.QMC_mx, p.qmc.QMC_my, p.qmc.QMC_mz,
                          // Madgwick
                          p.madgwick.q0, p.madgwick.q1, p.madgwick.q2, p.madgwick.q3, p.madgwick.beta,
                          // BMP180
                          p.bmp.timestamp, p.bmp.temp, p.bmp.pressure, p.bmp.altitude,
                          // BNO055
                          p.bno.timestamp, p.bno.BNO_ax, p.bno.BNO_ay, p.bno.BNO_az, p.bno.BNO_gx, p.bno.BNO_gy, p.bno.BNO_gz, p.bno.BNO_mx, p.bno.BNO_my, p.bno.BNO_mz, p.bno.BNO_qw, p.bno.BNO_qx, p.bno.BNO_qy, p.bno.BNO_qz, p.bno.BNO_global_ax, p.bno.BNO_global_ay, p.bno.BNO_global_az,
                          // Filter & Status
                          p.filter.filteredAltitude, p.filter.verticalVelocity, p.filter.verticalAccel, p.filter.alpha,
                          ok ? 1 : 0);

            if (!ok) countCorrupt++;
            countFast++;
            currentAddr += sizeof(FastFlightPacket);
        } 
        else if (id == 0x02) {
            SlowFlightPacket s;
            flashRead(currentAddr, (uint8_t*)&s, sizeof(SlowFlightPacket));
            
            uint16_t crc_calc = crc16CCITT((uint8_t*)&s, sizeof(SlowFlightPacket) - 2);
            bool ok = (s.checksum == crc_calc);

            Serial.printf("SLOW,%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%d,%d,%d,%.6lf,%.6lf,%.2f,%.2f,%.2f,%d,%.2f,%d,%d\n",
                            s.timestamp_ms,
                            s.bme.temp, s.bme.humidity, s.bme.pressure, s.bme.altitude,
                            s.bmpData.temp, s.bmpData.pressure, s.bmpData.altitude,
                            s.gps.year, s.gps.month, s.gps.day, s.gps.hour, s.gps.minute, s.gps.second,
                            s.gps.latitude, s.gps.longitude, s.gps.altitude, s.gps.speed, s.gps.course,
                            s.gps.satellites, s.gps.hdop, s.gps.valid ? 1 : 0,
                            ok ? 1 : 0);

            if (!ok) countCorrupt++;
            countSlow++;
            currentAddr += sizeof(SlowFlightPacket);
        } 
        else {
            Serial.printf("[?] Unknown byte in 0x%06lX: 0x%02X (Saltando...)\n", currentAddr, id);
            currentAddr++; 
            countCorrupt++;
        }

        if ((countFast + countSlow) % 50 == 0) delay(5);
    }

    println("Verification results");
    Serial.printf("  - Fast Packets: %lu\n", countFast);
    Serial.printf("  - Slow Packets: %lu\n", countSlow);
    Serial.printf("  - Corrupted Bytes: %lu\n", countCorrupt);
    println("Verification end");
}