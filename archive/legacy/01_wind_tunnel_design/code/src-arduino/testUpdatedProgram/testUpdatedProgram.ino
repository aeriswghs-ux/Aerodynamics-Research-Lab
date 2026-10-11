#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA 1013.25

// SD Card CS (Chip Select) pin
#define SD_CS_PIN 10 

// MPXV7002 Configuration
#define MPX_PIN A1            // Analog pin connected to MPXV7002 VOUT (mapped to diagram)
#define ARDUINO_VCC 5.0F      // Arduino supply voltage (typically 5.0V for Uno/Mega)
#define ADC_RESOLUTION 1023.0F // 10-bit ADC resolution for standard Arduino boards

// MPXV7002 modeling / calibration
// Datasheet relationship (normalized): Vout/Vcc = 0.2 * P(kPa) + 0.5
// Therefore P(kPa) = (Vout/Vcc - 0.5) / 0.2
const float MPXV_SENS_KPA = 0.2F;   // Vout/Vcc per kPa
const float MPXV_OFFSET = 0.5F;    // normalized offset
const float MPXV_MIN_KPA = -2.0F;  // sensor typical negative range (safety clamp)
const float MPXV_MAX_KPA = 2.0F;   // sensor typical positive range (safety clamp)
const float MPXV_NOISE_KPA = 0.005F; // simulate small sensor noise in kPa (~5 Pa)

// Op-amp conditioning model (simulate single-supply op-amp stage)
const float OPAMP_GAIN = 2.5F;        // non-inverting gain
const float OPAMP_OFFSET_KPA = 0.0F;  // output offset in kPa-equivalent
const float OPAMP_RAIL_POS = 5.0F;    // positive supply rail (V)
const float OPAMP_RAIL_NEG = 0.0F;    // negative supply rail (V)
const float OPAMP_OUTPUT_SAT_MARGIN = 0.05F; // margin from rails for saturation (V)

// Helper: simulate op-amp's conditioned output (input in Pa -> output in Pa)
float applyOpAmpConditioning(float rawPa) {
  // Convert Pa -> kPa for calibration steps
  float rawKpa = rawPa / 1000.0F;

  // Apply gain and offset (modeling a non-inverting amplifier with DC offset)
  float conditionedKpa = rawKpa * OPAMP_GAIN + OPAMP_OFFSET_KPA;

  // Convert back to Pa
  float conditionedPa = conditionedKpa * 1000.0F;

  // Simulate output rail saturation by mapping to a notional Vout, clamping, and remapping
  float vout = (conditionedKpa * MPXV_SENS_KPA) + MPXV_OFFSET; // notional normalized Vout
  float voutMax = OPAMP_RAIL_POS - OPAMP_OUTPUT_SAT_MARGIN;
  float voutMin = OPAMP_RAIL_NEG + OPAMP_OUTPUT_SAT_MARGIN;
  if (vout > voutMax) vout = voutMax;
  if (vout < voutMin) vout = voutMin;

  // Recompute conditionedPa from saturated Vout
  float finalKpa = (vout - MPXV_OFFSET) / MPXV_SENS_KPA;
  float finalPa = finalKpa * 1000.0F;
  return finalPa;
}

// Logging settings
#define LOG_FILENAME "logdata.csv" // Using 8.3 filename standard for SD library compatibility
#define LOG_INTERVAL 2000          // Milliseconds between logs

Adafruit_BME280 bme; // BME280 over I2C
bool sdCardAvailable = false;
unsigned long lastLogTime = 0;
unsigned long logCount = 0;

struct SensorReading {
  float temperatureC;
  float humidityPct;
  float pressureHpa;
  float altitudeM;
  float diffPressurePa; // Added MPXV7002 differential pressure in Pascals
  float diffPressureCondPa; // Conditioned (op-amp output) in Pascals
};

void printBanner();
void printPinInfo();
void i2cScan();
void printSensorIdGuide();
void printWiringHelp();
void configureBme280();
bool initSdCard();
bool ensureLogFileWithHeader();
bool initBme280();
bool readSensors(SensorReading &reading);
void printReading(unsigned long currentTime, const SensorReading &reading);
bool appendReadingToSd(unsigned long currentTime, const SensorReading &reading);
void printStartupSummary();

void setup() {
  Serial.begin(9600);
  // Avoid blocking on boards without native USB Serial (e.g. Uno)
  delay(200);

  printBanner();

  // Initialize I2C on default board pins.
  Wire.begin();
  // Seed PRNG for simulated sensor noise
  randomSeed(micros());
  printPinInfo();

  sdCardAvailable = initSdCard();

  Serial.println(F("Scanning I2C bus..."));
  i2cScan();
  Serial.println();

  if (!initBme280()) {
    while (1) {
      delay(10);
    }
  }

  configureBme280();
  printStartupSummary();
  delay(1000);
}

void loop() {
  const unsigned long currentTime = millis();

  // Time-sliced logging keeping loop unblocked and responsive
  if (currentTime - lastLogTime >= LOG_INTERVAL) {
    lastLogTime = currentTime;
    logCount++;

    SensorReading reading;
    if (!readSensors(reading)) {
      Serial.println(F("ERROR: Failed to read from sensors (NaN detected)."));
      return;
    }

    printReading(currentTime, reading);

    if (sdCardAvailable) {
      if (appendReadingToSd(currentTime, reading)) {
        Serial.println(F("  [OK] Logged to SD card"));
      } else {
        Serial.println(F("  [WARN] Error writing log file, disabling SD logging"));
        sdCardAvailable = false;
      }
    }

    Serial.println(F("----------------------------"));
  }
}

void printBanner() {
  Serial.println(F("BME280 + MPXV7002 + SD Card Logger"));
  Serial.println(F("=================================="));
  Serial.println();
}

void printPinInfo() {
  Serial.println(F("I2C Pins (Arduino Uno/R4):"));
  Serial.println(F("  SDA = A4"));
  Serial.println(F("  SCL = A5"));
  Serial.println(F("Analog Pins:"));
  Serial.println(F("  MPXV7002 VOUT = A1"));
  Serial.println();
}

bool initSdCard() {
  // Ensure the SD CS pin is set as OUTPUT to keep the SPI bus configured
  pinMode(SD_CS_PIN, OUTPUT);

  Serial.print(F("Initializing SD card on pin "));
  Serial.print(SD_CS_PIN);
  Serial.println(F("..."));

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println(F("[WARN] SD card initialization failed."));
    Serial.println(F("  Data will only display on Serial Monitor."));
    Serial.println();
    return false;
  }

  Serial.println(F("[OK] SD card initialized successfully."));
  if (!ensureLogFileWithHeader()) {
    Serial.println(F("[WARN] SD logging disabled because log file setup failed."));
    Serial.println();
    return false;
  }

  Serial.println();
  return true;
}

bool ensureLogFileWithHeader() {
  if (SD.exists(LOG_FILENAME)) {
    Serial.println(F("[OK] Appending to existing log file."));
    return true;
  }

  Serial.println(F("Creating new log file..."));
  File dataFile = SD.open(LOG_FILENAME, FILE_WRITE);
  if (!dataFile) {
    Serial.println(F("[WARN] Could not create log file."));
    return false;
  }

  // Updated CSV Header structure to include raw and conditioned differential pressure columns
  dataFile.println(F("Timestamp(ms),Temperature(C),Humidity(%),Pressure(hPa),Altitude(m),DiffPressureRaw(Pa),DiffPressureCond(Pa)"));
  dataFile.close();
  Serial.println(F("[OK] Log file created with header."));
  return true;
}

bool initBme280() {
  Serial.println(F("Attempting to connect to BME280..."));

  bool status = bme.begin(0x76);
  if (!status) {
    Serial.println(F("Not found at 0x76, trying 0x77..."));
    status = bme.begin(0x77);
  }

  if (!status) {
    Serial.println();
    printSensorIdGuide();
    printWiringHelp();
    return false;
  }

  Serial.println(F("[OK] BME280 detected successfully."));
  Serial.print(F("  Sensor ID: 0x"));
  Serial.println(bme.sensorID(), 16);
  Serial.println();
  return true;
}

void configureBme280() {
  bme.setSampling(
    Adafruit_BME280::MODE_NORMAL,
    Adafruit_BME280::SAMPLING_X2,   
    Adafruit_BME280::SAMPLING_X16,  
    Adafruit_BME280::SAMPLING_X1,   
    Adafruit_BME280::FILTER_X16,
    Adafruit_BME280::STANDBY_MS_500
  );
}

bool readSensors(SensorReading &reading) {
  // 1. Fetch BME280 Data
  reading.temperatureC = bme.readTemperature();
  reading.humidityPct = bme.readHumidity();
  reading.pressureHpa = bme.readPressure() / 100.0F;
  reading.altitudeM = bme.readAltitude(SEALEVELPRESSURE_HPA);

  // 2. Fetch MPXV7002 Differential Pressure Data
  int rawADC = analogRead(MPX_PIN);
  
  // Turn raw counts into actual voltage
  float voltage = ((float)rawADC / ADC_RESOLUTION) * ARDUINO_VCC;
  // Datasheet formula (normalized): Vout/Vcc = MPXV_SENS_KPA * P(kPa) + MPXV_OFFSET
  // Solving for P: P(kPa) = (Vout/Vcc - MPXV_OFFSET) / MPXV_SENS_KPA
  float pressureKpa = ((voltage / ARDUINO_VCC) - MPXV_OFFSET) / MPXV_SENS_KPA;

  // Add small simulated noise and clamp to sensor range
  float noise = ((float)random(-100, 101) / 100.0F) * MPXV_NOISE_KPA; // ±MPXV_NOISE_KPA
  pressureKpa += noise;
  if (pressureKpa < MPXV_MIN_KPA) pressureKpa = MPXV_MIN_KPA;
  if (pressureKpa > MPXV_MAX_KPA) pressureKpa = MPXV_MAX_KPA;

  // Scale kPa into standard Pascals (Pa)
  reading.diffPressurePa = pressureKpa * 1000.0F;

  // Apply op-amp conditioning model
  reading.diffPressureCondPa = applyOpAmpConditioning(reading.diffPressurePa);

  // Verify that data reads didn't result in an absolute error state
  return !(isnan(reading.temperatureC) || isnan(reading.humidityPct) ||
           isnan(reading.pressureHpa) || isnan(reading.altitudeM));
}

void printReading(unsigned long currentTime, const SensorReading &reading) {
  Serial.print(F("Log #"));
  Serial.print(logCount);
  Serial.print(F(" | Time: "));
  Serial.print(currentTime / 1000);
  Serial.println(F("s"));

  Serial.print(F("  Temperature: "));
  Serial.print(reading.temperatureC, 2);
  Serial.println(F(" C"));

  Serial.print(F("  Humidity:    "));
  Serial.print(reading.humidityPct, 2);
  Serial.println(F(" %"));

  Serial.print(F("  Pressure:    "));
  Serial.print(reading.pressureHpa, 2);
  Serial.println(F(" hPa"));

  Serial.print(F("  Altitude:    "));
  Serial.print(reading.altitudeM, 2);
  Serial.println(F(" m"));

  Serial.print(F("  Diff Press:  "));
  Serial.print(reading.diffPressurePa, 2);
  Serial.println(F(" Pa"));

  Serial.print(F("  Diff Press (cond):  "));
  Serial.print(reading.diffPressureCondPa, 2);
  Serial.println(F(" Pa"));
}

bool appendReadingToSd(unsigned long currentTime, const SensorReading &reading) {
  File dataFile = SD.open(LOG_FILENAME, FILE_WRITE);
  if (!dataFile) {
    return false;
  }

  // Formatting layout: timestamp,temp,humidity,pressure,altitude,diff_pressure
  dataFile.print(currentTime);
  dataFile.print(",");
  dataFile.print(reading.temperatureC, 2);
  dataFile.print(",");
  dataFile.print(reading.humidityPct, 2);
  dataFile.print(",");
  dataFile.print(reading.pressureHpa, 2);
  dataFile.print(",");
  dataFile.print(reading.altitudeM, 2);
  dataFile.print(",");
  dataFile.print(reading.diffPressurePa, 2); // Appends the raw differential pressure
  dataFile.print(",");
  dataFile.println(reading.diffPressureCondPa, 2); // Appends the conditioned (op-amp) differential pressure
  dataFile.close();

  return true;
}

void printStartupSummary() {
  Serial.println(F("Starting measurements..."));
  Serial.print(F("Interval: "));
  Serial.print(LOG_INTERVAL);
  Serial.println(F(" ms"));

  if (sdCardAvailable) {
    Serial.print(F("Logging to SD card: "));
    Serial.println(LOG_FILENAME);
  } else {
    Serial.println(F("SD logging: disabled"));
  }
  Serial.println();
}

void printSensorIdGuide() {
  Serial.println(F("Sensor ID Guide:"));
  Serial.println(F("  0x00 = No device (likely wiring/power issue)"));
  Serial.println(F("  0x60 = BME280"));
  Serial.println();
}

void printWiringHelp() {
  Serial.println(F("Wiring Help:"));
  Serial.println(F("  BME280 SDA -> A4 | SCL -> A5"));
  Serial.println(F("  MPXV7002 VOUT -> A1"));
  Serial.println();
}

void i2cScan() {
  byte error, address;
  int deviceCount = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print(F("  Found device at 0x"));
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println();
      deviceCount++;
    }
  }
  if (deviceCount == 0) Serial.println(F("  No I2C devices found!"));
}