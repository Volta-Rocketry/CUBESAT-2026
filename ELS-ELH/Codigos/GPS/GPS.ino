#include <TinyGPSPlus.h>

#define UBLOX_TX 25  ///< Transmission pin (TX) to the GPS
#define UBLOX_RX 36
#define GPS_BAUD 9600

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

struct StructUblox {
    uint32_t timestamp;   ///< Local system timestamp in ms
    
    // GPS Time
    uint8_t hour;         ///< UTC Hour
    uint8_t minute;       ///< UTC Minute
    uint8_t second;       ///< UTC Second
    
    // GPS Date
    uint16_t year;        ///< Year
    uint8_t month;        ///< Month
    uint8_t day;          ///< Day
    
    double latitude;      ///< Latitude in decimal degrees
    double longitude;     ///< Longitude in decimal degrees
    float altitude;       ///< Altitude above mean sea level in meters
    float speed;          ///< Ground speed
    float course;         ///< Course over ground
    uint8_t satellites;   ///< Number of satellites
    float hdop;           ///< Horizontal Dilution of Precision
    bool valid;           ///< GPS data validity flag
};

StructUblox ubloxData;

void InitUblox() {
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, UBLOX_RX, UBLOX_TX);

    unsigned long startTime = millis();
    bool GPSInitialized = false;
    while (gpsSerial.available() > 0) {
        gpsSerial.read();
    }
    while (millis() - startTime < 3000) {
        if (gpsSerial.available() > 0) {
            char c = gpsSerial.read(); 
            if (c == '$') { 
                GPSInitialized = true; 
                break;
            }
        }
    }
    if (!GPSInitialized) {
        Serial.println("Ublox initialization failed");
    } else {
        Serial.println("Ublox sensor initialized successfully");
    }
}

void CalibrateSensors() {
  bool GPSConected = false;
    // GPS connection check
    gpsSerial.begin(GPS_BAUD,SERIAL_8N1,UBLOX_RX,UBLOX_TX);
    while(!GPSConected){
      Serial.println("Calinrando");
      while (gpsSerial.available() > 0) {
          gps.encode(gpsSerial.read());
      }
      if (!GPSConected && gps.location.isValid() && gps.satellites.value() > 3) {
          GPSConected = true;
          Serial.println("Calibrado");
      }
    }
}
void ReadUblox() {
    // Code to read data from Ublox sensor
    ubloxData.timestamp = millis();
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }
    if (gps.time.isUpdated()) {
        ubloxData.hour = gps.time.hour();
        ubloxData.minute = gps.time.minute();
        ubloxData.second = gps.time.second();
    }
    if (gps.date.isUpdated()) {
        ubloxData.year = gps.date.year();
        ubloxData.month = gps.date.month();
        ubloxData.day = gps.date.day();
    }
    if (gps.location.isUpdated()) {
        ubloxData.latitude = gps.location.lat();
        ubloxData.longitude = gps.location.lng();
    }
    if (gps.altitude.isUpdated()) {
        ubloxData.altitude = gps.altitude.meters();
    }
    if (gps.speed.isUpdated()) {
        ubloxData.speed = gps.speed.mps();
    }
    if (gps.course.isUpdated()) {
        ubloxData.course = gps.course.deg();
    }
    if (gps.satellites.isUpdated()) {
        ubloxData.satellites = gps.satellites.value();
    }
    if (gps.hdop.isUpdated()) {
        ubloxData.hdop = gps.hdop.hdop();
    }
    ubloxData.valid = gps.location.isValid();

  
  // ===== SERIAL PRINT =====

  Serial.println("===== UBLOX GPS =====");

  Serial.print("Timestamp: ");
  Serial.println(ubloxData.timestamp);

  Serial.print("Time: ");
  Serial.print(ubloxData.hour);
  Serial.print(":");
  Serial.print(ubloxData.minute);
  Serial.print(":");
  Serial.println(ubloxData.second);

  Serial.print("Date: ");
  Serial.print(ubloxData.day);
  Serial.print("/");
  Serial.print(ubloxData.month);
  Serial.print("/");
  Serial.println(ubloxData.year);

  Serial.print("Latitude: ");
  Serial.println(ubloxData.latitude, 6);

  Serial.print("Longitude: ");
  Serial.println(ubloxData.longitude, 6);

  Serial.print("Altitude: ");
  Serial.print(ubloxData.altitude);
  Serial.println(" m");

  Serial.print("Speed: ");
  Serial.print(ubloxData.speed);
  Serial.println(" m/s");

  Serial.print("Course: ");
  Serial.print(ubloxData.course);
  Serial.println(" deg");

  Serial.print("Satellites: ");
  Serial.println(ubloxData.satellites);

  Serial.print("HDOP: ");
  Serial.println(ubloxData.hdop);

  Serial.print("Valid: ");
  Serial.println(ubloxData.valid);

  Serial.println("=====================");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  InitUblox();


}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);

}
