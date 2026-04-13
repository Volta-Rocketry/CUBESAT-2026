#include "flight_computer.h"

#include "constants.h"
#include "sensors_manager.h"    
#include "error_warning.h"      
#include "flash_storage.h"    
#include <Arduino.h>
#include <SD.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

static FlightState g_state = STATE_INIT;
static uint32_t g_flash_write_addr = 0;
static uint8_t g_page_buf[FLASH_PAGE_SIZE];
static uint16_t g_page_buf_idx = 0;

FastFlightPacket FFP;
SlowFlightPacket SFP;

StructMPU9250 mpuData;
StructBNO055 bnoData;
StructBME280 bmeData;
StructUblox ubloxData;
StructTransducer transducerData;

static uint16_t crc16(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i] << 8;   /* Mezcla el byte con el byte alto del CRC */
        for (int b = 0; b < 8; b++) {
            /* Si bit 15 = 1: desplaza y aplica el polinomio (reducción modular).
             * Si bit 15 = 0: solo desplaza.                                 */
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
        }
    }
    return crc;
}

static void page_buf_flush() {
    if (g_page_buf_idx == 0) return;

    if (g_flash_write_addr + g_page_buf_idx > FLASH_TOTAL_BYTES) {
        Serial.println("[DAQ] Flash llena. Deteniendo grabacion.");
        return;
    }

    flash_write(g_flash_write_addr, g_page_buf, g_page_buf_idx);
    g_flash_write_addr += g_page_buf_idx;
    g_page_buf_idx      = 0;
}

static void page_buf_write(const uint8_t* data, uint16_t len) {
    uint16_t written = 0;
    while (written < len) {
        uint16_t space = FLASH_PAGE_SIZE - g_page_buf_idx;
        uint16_t to_copy = (len - written) < space ? (len - written) : space;

        memcpy(g_page_buf + g_page_buf_idx, data + written, to_copy);
        g_page_buf_idx += to_copy;
        written += to_copy;

        if (g_page_buf_idx == FLASH_PAGE_SIZE) {
            page_buf_flush();
        }
    }
}

static void record_fast_packet() {
    FFP.packet_id = 0x01;
    FFP.timestamp_ms = millis();
    FFP.mpu = mpuData; 
    FFP.bno = bnoData;
    FFP.transducer = transducerData;

    FFP.checksum = crc16((uint8_t*)&FFP, sizeof(FastFlightPacket) - sizeof(uint16_t));

    page_buf_write((uint8_t*)&FFP, sizeof(FastFlightPacket));
}

static void record_slow_packet() {
    SFP.packet_id = 0x02;
    SFP.timestamp_ms = millis();
    SFP.bme = bmeData;
    SFP.gps = ubloxData;

    SFP.checksum = crc16((uint8_t*)&SFP, sizeof(SlowFlightPacket) - sizeof(uint16_t));

    page_buf_write((uint8_t*)&SFP, sizeof(SlowFlightPacket));
}

static void download_flash_to_sd() {

    if (g_flash_write_addr == 0) {
        Serial.println("[DOWNLOAD] Flash vacia.");
        return;
    }

    File fast_file = SD.open("/fast_log.csv", FILE_WRITE);
    File slow_file = SD.open("/slow_log.csv", FILE_WRITE);

    if (!fast_file) {
        CriticalErrorSensor("No se pudo abrir fast_data.csv en SD");
        return;
    }
    if (!slow_file) {
        CriticalErrorSensor("No se pudo abrir slow_data.csv en SD");
        return;
    }

    fast_file.println(
        "timestamp_ms,"
        "MPU_ax,MPU_ay,MPU_az,MPU_gx,MPU_gy,MPU_gz,MPU_mx,MPU_my,MPU_mz,"
        "BNO_ax,BNO_ay,BNO_az,BNO_gx,BNO_gy,BNO_gz,BNO_mx,BNO_my,BNO_mz,"
        "transducer_voltage,transducer_pressure,"
        "crc_ok"
    );
    slow_file.println(
        "timestamp_ms,"
        "bme_temp,bme_humidity,bme_pressure,bme_altitude,"
        "gps_lat,gps_lon,gps_alt,gps_speed,"
        "crc_ok"
    );

    uint32_t     current_addr  = 0;
    uint32_t     corrupt_count = 0;

    while (current_addr < g_flash_write_addr) {

        uint8_t id;
        flash_read(current_addr, &id, 1);

        if (id == 0x01) {
            FastFlightPacket fast_pkt;

            flash_read(current_addr, (uint8_t*)&fast_pkt, sizeof(FastFlightPacket));

            uint16_t expected_crc = crc16((uint8_t*)&fast_pkt, sizeof(FastFlightPacket) - sizeof(uint16_t));
            bool crc_ok = (fast_pkt.checksum == expected_crc);
            if (!crc_ok) corrupt_count++;

            char csv_line[256];
            snprintf(csv_line, sizeof(csv_line), 
                "%lu,"
                "%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,"
                "%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,"
                "%.4f,%.4f,"
                "%s",
                (unsigned long)fast_pkt.timestamp_ms,
                fast_pkt.mpu.MPU_ax, fast_pkt.mpu.MPU_ay, fast_pkt.mpu.MPU_az,
                fast_pkt.mpu.MPU_gx, fast_pkt.mpu.MPU_gy, fast_pkt.mpu.MPU_gz,
                fast_pkt.mpu.MPU_mx, fast_pkt.mpu.MPU_my, fast_pkt.mpu.MPU_mz,
                fast_pkt.bno.BNO_ax, fast_pkt.bno.BNO_ay, fast_pkt.bno.BNO_az,
                fast_pkt.bno.BNO_gx, fast_pkt.bno.BNO_gy, fast_pkt.bno.BNO_gz,
                fast_pkt.bno.BNO_mx, fast_pkt.bno.BNO_my, fast_pkt.bno.BNO_mz,
                fast_pkt.transducer.voltage,
                fast_pkt.transducer.pressureTransducer,
                crc_ok ? "OK" : "CORRUPT"
            );
            fast_file.println(csv_line);

            current_addr += sizeof(FastFlightPacket);
        }

        else if (id == 0x02) {
            SlowFlightPacket slow_pkt;
            flash_read(current_addr, (uint8_t*)&slow_pkt, sizeof(SlowFlightPacket));

            uint16_t expected_crc = crc16((uint8_t*)&slow_pkt, sizeof(SlowFlightPacket) - sizeof(uint16_t));
            bool crc_ok = (slow_pkt.checksum == expected_crc);
            if (!crc_ok) corrupt_count++;

            char csv_line[192];
            snprintf(csv_line, sizeof(csv_line), 
                "%lu,%.2f,%.2f,%.2f,%.2f,%.6f,%.6f,%.2f,%.2f,%s",
                (unsigned long)slow_pkt.timestamp_ms,
                slow_pkt.bme.temp,
                slow_pkt.bme.humidity,
                slow_pkt.bme.pressure,
                slow_pkt.bme.altitude,
                slow_pkt.gps.latitude,
                slow_pkt.gps.longitude,
                slow_pkt.gps.altitude,
                slow_pkt.gps.speed,
                crc_ok ? "OK" : "CORRUPT"
            );    
            slow_file.println(csv_line);

            current_addr += sizeof(SlowFlightPacket);
        }

        else {
            current_addr++;
            corrupt_count++;
        }

    }

    fast_file.close();
    slow_file.close();

    Serial.printf("[DOWNLOAD] Listo. Bytes leidos: %lu. Errores: %lu\n",
        (unsigned long)g_flash_write_addr,
        (unsigned long)corrupt_count
    );

    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_BLUE_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, HIGH);
}

void flight_computer_init() {

    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_BLUE_PIN, LOW);

    pinMode(PIN_BUTTON, INPUT_PULLUP); // se puede agregar a la inicializaion de los leds

    if (!flash_init()) {
        CriticalErrorSensor("W25Q128 no detectada. Verificar HSPI y FLASH_CS.");
    }
    Serial.println("[DAQ] Flash W25Q128 OK.");

    if (!SD.begin(PIN_CS_SD)) {
        Serial.println("[DAQ] ADVERTENCIA: SD no detectada.");
    } 
    else {
        Serial.println("[DAQ] SD OK.");
    }

    Serial.println("[DAQ] Borrando flash (esperar)...");
    flash_erase_chip();
    Serial.println("[DAQ] Flash borrada OK.");

    g_flash_write_addr = 0;
    g_page_buf_idx = 0;

    Serial.printf("[DAQ] Flash: %lu MB disponibles.\n", 
        FLASH_TOTAL_BYTES / (1024UL * 1024UL));
    Serial.printf("[DAQ] Estimacion: ~%.1f min de vuelo (FastPackets a 100Hz)\n", 
        (float)(FLASH_TOTAL_BYTES / sizeof(FastFlightPacket)) / 6000.0f);

    digitalWrite(LED_BLUE_PIN, HIGH);
    g_state = STATE_ASCENT; //Para prueba (cambiar a STATE_PAD para vuelo)
    Serial.println("[DAQ] MODO PRUEBA: grabando en STATE_ASCENT. Presionar boton para descargar.");
}

void flight_computer_update() {

    static uint32_t last_fast_sample = 0;
    static uint32_t last_slow_sample = 0;
    static uint32_t accel_start_ms   = 0; 
    static float    max_altitude     = -999.0f;
    static uint8_t  apogee_count     = 0;
    static uint32_t stable_start_ms  = 0;
    static uint32_t landed_start_ms  = 0;
    static float    last_landed_alt  = 0.0f;

    uint32_t now = millis();

    switch (g_state) {

    case STATE_IDLE: 
        // Los print deben ser en el bluethooth, ademas sugerencias de que mas comandos se puedan agregar en la prueba
        digitalWrite(LED_BLUE_PIN, HIGH);

        CloseActuatorsVoltage();

        if (now - last_slow_sample >= 1000) {
            last_slow_sample = now;
            ReadBME280();
            ReadUblox();
            ReadMPU9250();
            ReadBNO055();

            Serial.printf("[IDLE] OK, Actuators inhibided, BME280 T: %.2fC P: %.2fPa A: %.2fm, Ublox Lat: %.6f Lon: %.6f Alt: %.2fm Spd: %.2fm/s, MPU9250 Accel: (%.2f, %.2f, %.2f) m/s², BNO055 Accel: (%.2f, %.2f, %.2f) m/s²\n",
                bmeData.temp, bmeData.pressure, bmeData.altitude,
                ubloxData.latitude, ubloxData.longitude, ubloxData.altitude, ubloxData.speed,
                mpuData.MPU_ax, mpuData.MPU_ay, mpuData.MPU_az,
                bnoData.BNO_ax, bnoData.BNO_ay, bnoData.BNO_az
            );  // se debe imprimir en el bluetooth

            if (Serial.available() > 0) {
                String cmd = Serial.readStringUntil('\n');
                cmd.trim();
                if (cmd == "INTEGRATION") {
                    Serial.println("[COMANDO] Pasando a INTEGRATION.");
                    g_state = STATE_INTEGRATION;
                    digitalWrite(LED_BLUE_PIN, LOW);
                    digitalWrite(LED_GREEN_PIN, HIGH);
                }
                else if (cmd == "SAVE SLOW DATA") {
                    Serial.println("[COMANDO] Guardando un paquete lento de prueba.");
                    record_slow_packet();
                }
                else if (cmd == "SAVE FAST DATA") {
                    Serial.println("[COMANDO] Guardando un paquete rapido de prueba.");
                    record_fast_packet();
                }
                else if (cmd == "DOWNLOAD") {
                    Serial.println("[COMANDO] Iniciando descarga por comando serial.");
                    download_flash_to_sd();
                }
                else if (cmd == "ERASE") {
                    Serial.println("[COMANDO] Borrando flash por comando serial.");
                    flash_erase_chip();
                    g_flash_write_addr = 0;
                    g_page_buf_idx = 0;
                }
                if (cmd == "PAD") {
                    Serial.println("[COMANDO] Pasando a PAD.");
                    g_state = STATE_PAD;
                    digitalWrite(LED_BLUE_PIN, LOW);
                    digitalWrite(LED_RED_PIN, HIGH);
                }
            }

            
        }
        
        break;

    case STATE_INIT:
        //arranque del sistema
        break;

    case STATE_INTEGRATION:
        if (now - last_slow_sample >= 5000) {
            last_slow_sample = now;
            ReadBME280();
            record_slow_packet();
        }
        break;

    case STATE_PAD:

        CloseActuatorsVoltage();

        if (now - last_slow_sample >= 10000) {
            last_slow_sample = now;

            ReadBME280();
            ReadUblox();
            record_slow_packet();
        }
        if (now - last_fast_sample >= FAST_SAMPLE_INTERVAL_MS) { 
            last_fast_sample = now;

            ReadMPU9250();
            ReadBME280(); 

            float total_accel = sqrtf(
                mpuData.MPU_ax * mpuData.MPU_ax +
                mpuData.MPU_ay * mpuData.MPU_ay +
                mpuData.MPU_az * mpuData.MPU_az
            );

            if (total_accel > LAUNCH_ACCEL_THRESHOLD_MS2) {
                if (accel_start_ms == 0) accel_start_ms = now;
                
                if ((now - accel_start_ms >= 500) && (bmeData.altitude > 30.0f)) {
                    Serial.println("[EVENTO] DESPEGUE DETECTADO. Pasando a ASCENT.");
                    g_state = STATE_ASCENT;
                    digitalWrite(LED_RED_PIN, LOW);
                    digitalWrite(LED_GREEN_PIN, HIGH);
                }
            } 
            else {
                accel_start_ms = 0;
            }
        }
        break;

    case STATE_ASCENT:
        if (now - last_fast_sample >= FAST_SAMPLE_INTERVAL_MS) {
            last_fast_sample = now;
            ReadMPU9250();
            ReadBNO055();
            ReadTransducers();
            record_fast_packet(); // 100 Hz
        }
        if (now - last_slow_sample >= SLOW_SAMPLE_INTERVAL_MS) {
            last_slow_sample = now;
            ReadBME280();
            ReadUblox();
            record_slow_packet(); // 1 Hz

            if (bmeData.altitude > max_altitude) {
                max_altitude = bmeData.altitude;
                apogee_count = 0;
            } 
            else if (bmeData.altitude < (max_altitude - 5.0f)) {
                apogee_count++;
                if (apogee_count >= 10) {
                    Serial.println("[EVENTO] APOGEO DETECTADO. Pasando a EJECTION.");
                    g_state = STATE_EYECTION;
                    digitalWrite(LED_GREEN_PIN, LOW);
                }
            } 
            else {
                apogee_count = 0;
            }
        }
        break;

    case STATE_EYECTION:
        if (now - last_fast_sample >= FAST_SAMPLE_INTERVAL_MS) {
            last_fast_sample = now;
            ReadMPU9250();
            ReadBNO055();
            ReadTransducers();
            record_fast_packet();

            float total_gyro = sqrtf(
                mpuData.MPU_gx * mpuData.MPU_gx +
                mpuData.MPU_gy * mpuData.MPU_gy +
                mpuData.MPU_gz * mpuData.MPU_gz
            );
            
            // Falta ademas comprobar que los kill switches dejaron de estar presionados
            if (total_gyro < 20.0f) {  // por que 20, cuando se estabilize no podria ser mayor?
                if (stable_start_ms == 0) stable_start_ms = now;
                if (now - stable_start_ms > 2000) {
                    Serial.println("[EVENTO] Cubesat estabilizado. Pasando a CONTROL.");
                    g_state = STATE_CONTROL;
                }
            } 
            else {
                stable_start_ms = 0;
            }
        }
        if (now - last_slow_sample >= SLOW_SAMPLE_INTERVAL_MS) {
            last_slow_sample = now;
            ReadBME280();
            ReadUblox();
            record_slow_packet();
        }
        break;

    case STATE_CONTROL:

        OpenActuatorsVoltage();

        if (now - last_fast_sample >= FAST_SAMPLE_INTERVAL_MS) {
            last_fast_sample = now;
            ReadMPU9250();
            ReadBNO055();
            ReadTransducers();
            record_fast_packet();
        }
        if (now - last_slow_sample >= SLOW_SAMPLE_INTERVAL_MS) {
            last_slow_sample = now;
            ReadBME280();
            ReadUblox();
            record_slow_packet();

            if (bmeData.altitude < 500.0f) {
                Serial.println("[EVENTO] Umbral de drenaje cruzado. Pasando a DRAIN.");
                g_state = STATE_DRAIN;
            }
        }
        break;

    case STATE_DRAIN:
        if (now - last_fast_sample >= FAST_SAMPLE_INTERVAL_MS) {
            last_fast_sample = now;
            ReadMPU9250();
            ReadBNO055();
            ReadTransducers();
            record_fast_packet();
        }
        if (now - last_slow_sample >= SLOW_SAMPLE_INTERVAL_MS) {
            last_slow_sample = now;
            ReadBME280();
            ReadUblox();
            record_slow_packet();

            if (fabsf(bmeData.altitude - last_landed_alt) < 2.0f) {
                if (landed_start_ms == 0) landed_start_ms = now;
                if (now - landed_start_ms > 5000) { 

                    page_buf_flush(); 
                    Serial.println("[EVENTO] ATERRIZAJE CONFIRMADO. Pasando a RECOVERY.");
                    g_state = STATE_RECOVERY;
                    digitalWrite(LED_RED_PIN, LOW);
                }
            }
            else {
                last_landed_alt = bmeData.altitude;
                landed_start_ms = 0;
            }
        }
        break;

    case STATE_RECOVERY:

        digitalWrite(LED_GREEN_PIN, (now % 1000) < 500 ? HIGH : LOW);

        if (now - last_slow_sample >= 10000) {
            last_slow_sample = now;
            ReadBME280(); 
        }

        if (digitalRead(PIN_BUTTON) == LOW) {
            delay(50); // No es muy poco tiempo?
            if (digitalRead(PIN_BUTTON) == LOW) {
                g_state = STATE_DOWNLOAD; 
                digitalWrite(LED_GREEN_PIN, LOW);
                digitalWrite(LED_BLUE_PIN, HIGH);
                Serial.println("[DAQ] Iniciando descarga a SD...");
            }
        }
        break;

    case STATE_DOWNLOAD:

        download_flash_to_sd();
        Serial.println("[DAQ] Descarga completa. Reiniciar para nuevo vuelo.");
        while (true) {
            digitalWrite(LED_GREEN_PIN, HIGH);
            delay(1000);
        }
        break;

    default:
        break;
    }
}

FlightState flight_computer_get_state() {
    return g_state;
}