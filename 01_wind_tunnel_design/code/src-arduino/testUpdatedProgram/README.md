# BME280 + MPXV7002 + SD Card Data Logger

This project is an Arduino-based environmental and aerodynamic data logging system. It reads data from a **BME280** sensor (temperature, humidity, atmospheric pressure, altitude) and an **MPXV7002** differential pressure sensor (simulated via a potentiometer and op-amp circuit in Wokwi). The compiled readings are displayed on the Serial Monitor and saved into a CSV file on a Micro SD card at regular intervals.

---

## 🛠️ Hardware & System Architecture

The project utilizes an **Arduino Uno** connected to three primary modules:
1. **BME280 Sensor:** Measures ambient temperature, relative humidity, barometric pressure, and estimates altitude. Communicates via the **I2C** bus.
2. **MPXV7002 Differential Pressure Sensor (Simulated):** Measures low-range air pressure differentials (commonly used in Pitot tube setups for airspeed calculation). 
   * *Simulation Details:* In Wokwi, this is modeled using a potentiometer feeding into an operational amplifier (`wokwi-op-amp`) to condition the signal before it reaches the Arduino's analog pin.
3. **Micro SD Card Module:** Provides non-volatile storage to log sensor telemetry over the **SPI** bus.

### 🔌 Pin Configuration Mapping

| Component | Component Pin | Arduino Pin | Protocol / Connection Type |
| :--- | :--- | :--- | :--- |
| **BME280** | VCC <br> GND <br> SDA <br> SCL | 5V <br> GND <br> A4 <br> A5 | I2C Power <br> I2C Power <br> I2C Data <br> I2C Clock |
| **MPXV7002 Circuit** | VCC <br> GND <br> Op-Amp OUT | 5V <br> GND <br> A1 | Analog Power <br> Analog Power <br> Analog Input |
| **Micro SD Card** | VCC <br> GND <br> CS <br> MOSI <br> MISO <br> SCK | 5V <br> GND <br> 10 <br> 11 <br> 12 <br> 13 | SPI Power <br> SPI Power <br> Chip Select <br> SPI Data In <br> SPI Data Out <br> SPI Clock |

---

## 💻 Software Overview

The firmware (`testUpdatedProgram.ino`) is written in C++ using the Arduino framework and operates using a non-blocking time-slice loop pattern.

### Key Logic & Features

* **Non-Blocking Execution:** Instead of utilizing standard `delay()` functions inside `loop()`, the script uses `millis()` to check if the `LOG_INTERVAL` (2000 ms) has passed. This ensures the microcontroller remains responsive to other events.
* **I2C Bus Diagnostics:** During the `setup()` sequence, an auto-scan functionality checks the I2C bus addresses (`0x76` or `0x77`) to verify whether the BME280 is physically connected.
* **Op-Amp Signal Conditioning Emulation:** The code contains a mathematical simulation (`applyOpAmpConditioning`) mimicking a hardware non-inverting amplifier. It scales, offsets, adds artificial sensor noise, and applies hardware rail-saturation limits ($0.05\text{V}$ to $4.95\text{V}$) to the differential pressure data.
* **Automatic CSV File Management:** Checks for the presence of `logdata.csv` on the SD Card. If it is missing, the software creates it and writes a structured header row. If it exists, it seamlessly appends new data rows.

---

## 📊 Telemetry Data Output

Every 2 seconds, the program generates data structures across the following fields:

* **Timestamp:** Elapsed runtime in milliseconds.
* **Temperature:** Ambient thermal reading in Celsius (°C).
* **Humidity:** Relative atmospheric moisture percentage (%).
* **Pressure:** Absolute barometric reading in Hectopascals (hPa).
* **Altitude:** Calculated height above sea level in meters (m).
* **DiffPressureRaw:** Unconditioned differential pressure in Pascals (Pa).
* **DiffPressureCond:** Op-amp enhanced/conditioned differential pressure in Pascals (Pa).

### Serial Monitor Example Preview
```text
Log #1 | Time: 2s
  Temperature: 24.50 C
  Humidity:    45.20 %
  Pressure:    1013.15 hPa
  Altitude:    0.85 m
  Diff Press:  12.40 Pa
  Diff Press (cond):  31.00 Pa
  [OK] Logged to SD card
----------------------------