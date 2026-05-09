#include <Adafruit_BME280.h>
Adafruit_BME280 bme(5);

struct StructBME280 {
    uint32_t timestamp;  ///< Reading timestamp in ms
    float temp;          ///< Temperature in Celsius degrees
    float humidity;      ///< Relative humidity in percentage (%)
    float pressure;      ///< Atmospheric pressure in Pa
    float altitude;      ///< Calculated altitude in meters
};

struct CalibrationDataBME {
    float bmePresRef;        ///< Reference pressure for altitude calculation
};

StructBME280 bmeData;
CalibrationDataBME bmeCalib;

void InitBME280() {
    // Code to initialize BME280 sensor
    if (!bme.begin()) {
        Serial.println("BME280 initialization failed");
    } else {
        Serial.println("BME280 sensor initialized successfully");
    }
}

void CalibrateSensors() {
    float gSumX = 0, gSumY = 0, gSumZ = 0;
    float aSumX = 0, aSumY = 0, aSumZ = 0;
    float tSum = 0;
    float pSumBME = 0;
    float pSumBMP = 0;
    bool GPSConected = false;
    int numReadings = 0;
    const int MAX_MU = 1000;
    uint32_t previousCalibMilis = millis();

    // Data collection loop for calibration
    Serial.println("Calibration loop.");
    while (numReadings < MAX_MU) {
        uint32_t calibMilis = millis();
        if (calibMilis - previousCalibMilis >= 150) {
            previousCalibMilis = calibMilis;

            pSumBME += bme.readPressure();

            numReadings++;
        }
    }

    bmeCalib.bmePresRef = pSumBME / float(numReadings);

}
void ReadBME280() {
    // Code to read data from BME sensor
    float pressurePad2 = bmeCalib.bmePresRef / 100.0f; 
    bmeData.timestamp = millis();
    bmeData.temp = bme.readTemperature();
    bmeData.humidity = bme.readHumidity();
    bmeData.pressure = bme.readPressure();
    bmeData.altitude = bme.readAltitude(pressurePad2);
    Serial.println("===== BME280 =====");

    Serial.print("Timestamp: ");
    Serial.println(bmeData.timestamp);

    Serial.print("Temperature: ");
    Serial.print(bmeData.temp);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(bmeData.humidity);
    Serial.println(" %");

    Serial.print("Pressure: ");
    Serial.print(bmeData.pressure);
    Serial.println(" Pa");

    Serial.print("Altitude: ");
    Serial.print(bmeData.altitude);
    Serial.println(" m");

    Serial.println("==================");
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  InitBME280();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
}
