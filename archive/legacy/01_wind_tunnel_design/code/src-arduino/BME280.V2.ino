#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA 1013.25

// SD Card CS (Chip Select) pin - CHANGE THIS to match your wiring
#define SD_CS_PIN 10  // Common: 10 for Uno, 53 for Mega

// Log file settings
#define LOG_FILENAME "bme280.csv"
#define LOG_INTERVAL 2000  // milliseconds between logs

Adafruit_BME280 bme; // I2C
bool sdCardAvailable = false;
unsigned long lastLogTime = 0;
unsigned long logCount = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(10);
  
  Serial.println(F("BME280 + SD Card Logger"));
  Serial.println(F("======================="));
  Serial.println();

  // IMPORTANT: Initialize I2C FIRST, before SD card
  Serial.println(F("Initializing I2C..."));
  Wire.begin();
  Wire.setClock(100000L);  // Set to 100kHz (slower, more stable)
  delay(100);  // Give I2C time to stabilize
  
  Serial.println(F("I2C Pins (Arduino Uno/R4):"));
  Serial.println(F("  SDA = A4"));
  Serial.println(F("  SCL = A5"));
  Serial.println(F("  I2C Speed = 100kHz (stable mode)"));
  Serial.println();

  // Scan for I2C devices BEFORE initializing SD
  Serial.println(F("Scanning I2C bus..."));
  i2cScan();
  Serial.println();

  // Try to initialize BME280 sensor BEFORE SD card
  Serial.println(F("Attempting to connect to BME280..."));
  bool status = bme.begin(0x76);
  if (!status) {
    Serial.println(F("Not found at 0x76, trying 0x77..."));
    status = bme.begin(0x77);
  }
  
  if (!status) {
    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F("ERROR: Could not find BME280 sensor!"));
    Serial.println(F("========================================"));
    Serial.println();
    
    Serial.print(F("Sensor ID: 0x"));
    Serial.println(bme.sensorID(), 16);
    Serial.println();
    
    Serial.println(F("Sensor ID Guide:"));
    Serial.println(F("  0x00 = No device (wiring issue)"));
    Serial.println(F("  0xFF = Not responding (check power)"));
    Serial.println(F("  0x56-0x58 = BMP280"));
    Serial.println(F("  0x60 = BME280 ✓"));
    Serial.println(F("  0x61 = BME680"));
    Serial.println();
    
    Serial.println(F("BME280 Wiring (I2C):"));
    Serial.println(F("  VCC -> 3.3V or 5V"));
    Serial.println(F("  GND -> GND"));
    Serial.println(F("  SDA -> A4 (with 4.7k pull-up to VCC)"));
    Serial.println(F("  SCL -> A5 (with 4.7k pull-up to VCC)"));
    Serial.println();
    
    Serial.println(F("Troubleshooting:"));
    Serial.println(F("  1. Check all connections are secure"));
    Serial.println(F("  2. Add 4.7k pull-up resistors to SDA & SCL"));
    Serial.println(F("  3. Try connecting to 3.3V instead of 5V"));
    Serial.println(F("  4. Make sure no other I2C devices conflict"));
    Serial.println(F("  5. Try a different BME280 module"));
    Serial.println();
    
    while (1) delay(10);
  }

  Serial.println(F("✓ BME280 detected successfully!"));
  Serial.print(F("  Sensor ID: 0x"));
  Serial.println(bme.sensorID(), 16);
  Serial.println();

  // Configure sensor
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X2,
                  Adafruit_BME280::SAMPLING_X16,
                  Adafruit_BME280::SAMPLING_X1,
                  Adafruit_BME280::FILTER_X16,
                  Adafruit_BME280::STANDBY_MS_500);

  // NOW initialize SD card (after BME280 is working)
  Serial.print(F("Initializing SD card on pin "));
  Serial.print(SD_CS_PIN);
  Serial.println(F("..."));
  
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("⚠ SD card initialization failed!"));
    Serial.println(F("  Data will only display on Serial Monitor."));
    Serial.println();
    Serial.println(F("SD Card Wiring (SPI):"));
    Serial.print(F("  CS   -> Pin "));
    Serial.println(SD_CS_PIN);
    Serial.println(F("  MOSI -> Pin 11"));
    Serial.println(F("  MISO -> Pin 12"));
    Serial.println(F("  SCK  -> Pin 13"));
    Serial.println(F("  VCC  -> 5V"));
    Serial.println(F("  GND  -> GND"));
    Serial.println();
    sdCardAvailable = false;
  } else {
    Serial.println(F("✓ SD card initialized successfully!"));
    sdCardAvailable = true;
    
    // Check if log file exists, if not create header
    if (!SD.exists(LOG_FILENAME)) {
      Serial.println(F("Creating new log file..."));
      File dataFile = SD.open(LOG_FILENAME, FILE_WRITE);
      if (dataFile) {
        dataFile.println(F("Timestamp(ms),Temperature(C),Humidity(%),Pressure(hPa),Altitude(m)"));
        dataFile.close();
        Serial.println(F("✓ Log file created with header"));
      } else {
        Serial.println(F("⚠ Could not create log file"));
        sdCardAvailable = false;
      }
    } else {
      Serial.println(F("✓ Appending to existing log file"));
    }
    Serial.println();
  }
                  
  Serial.println(F("Starting measurements..."));
  if (sdCardAvailable) {
    Serial.print(F("Logging to SD card: "));
    Serial.println(LOG_FILENAME);
  }
  Serial.println();
  delay(1000);
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check if it's time to log
  if (currentTime - lastLogTime >= LOG_INTERVAL) {
    lastLogTime = currentTime;
    logCount++;
    
    // Read sensor values
    float temp = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;
    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

    // Check if readings are valid
    if (isnan(temp) || isnan(humidity) || isnan(pressure)) {
      Serial.println(F("ERROR: Failed to read from sensor!"));
      return;
    }

    // Display on Serial Monitor
    Serial.print(F("Log #"));
    Serial.print(logCount);
    Serial.print(F(" | Time: "));
    Serial.print(currentTime / 1000);
    Serial.println(F("s"));
    
    Serial.print(F("  Temperature: "));
    Serial.print(temp, 2);
    Serial.println(F(" °C"));

    Serial.print(F("  Humidity:    "));
    Serial.print(humidity, 2);
    Serial.println(F(" %"));

    Serial.print(F("  Pressure:    "));
    Serial.print(pressure, 2);
    Serial.println(F(" hPa"));

    Serial.print(F("  Altitude:    "));
    Serial.print(altitude, 2);
    Serial.println(F(" m"));

    // Log to SD card if available
    if (sdCardAvailable) {
      File dataFile = SD.open(LOG_FILENAME, FILE_WRITE);
      
      if (dataFile) {
        // Write CSV line: timestamp,temp,humidity,pressure,altitude
        dataFile.print(currentTime);
        dataFile.print(",");
        dataFile.print(temp, 2);
        dataFile.print(",");
        dataFile.print(humidity, 2);
        dataFile.print(",");
        dataFile.print(pressure, 2);
        dataFile.print(",");
        dataFile.println(altitude, 2);
        dataFile.close();
        
        Serial.println(F("  ✓ Logged to SD card"));
      } else {
        Serial.println(F("  ⚠ Error opening log file"));
        sdCardAvailable = false;  // Stop trying if SD fails
      }
    }
    
    Serial.println(F("----------------------------"));
  }
}

// I2C Scanner function
void i2cScan() {
  byte error, address;
  int deviceCount = 0;

  Serial.println(F("  Scanning addresses 0x01-0x7F..."));
  
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print(F("  ✓ Found device at 0x"));
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      
      Serial.print(F(" ("));
      if (address == 0x76 || address == 0x77) {
        Serial.print(F("BME280/BMP280"));
      } else if (address == 0x68) {
        Serial.print(F("MPU6050/DS1307"));
      } else if (address == 0x3C || address == 0x3D) {
        Serial.print(F("OLED"));
      } else {
        Serial.print(F("Unknown"));
      }
      Serial.println(F(")"));
      
      deviceCount++;
    } else if (error == 4) {
      Serial.print(F("  ⚠ Unknown error at 0x"));
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  
  if (deviceCount == 0) {
    Serial.println(F("  ✗ No I2C devices found!"));
    Serial.println(F("  → Check SDA/SCL connections"));
    Serial.println(F("  → Add 4.7k pull-up resistors"));
    Serial.println(F("  → Verify power to sensor"));
  } else {
    Serial.print(F("  Total devices found: "));
    Serial.println(deviceCount);
  }
}