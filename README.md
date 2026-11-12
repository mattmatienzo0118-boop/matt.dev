# matt.dev
# TRIOE Environmental Monitoring Kit

### BME280 + MQ-135 | ESP32-S3 | Round LCD | IoT Web Dashboard

An ESP32-S3-based environmental monitoring platform featuring real-time sensor monitoring, a customizable LCD interface, a web-based dashboard, and a modular 3D-printed enclosure.

![Project Banner](assets/banner/project-banner.png)

<div align="center">

![ESP32](https://img.shields.io/badge/Controller-ESP32--S3-blue)
![BME280](https://img.shields.io/badge/Sensor-BME280-green)
![MQ135](https://img.shields.io/badge/Sensor-MQ--135-orange)
![IoT](https://img.shields.io/badge/Technology-IoT-cyan)
![Status](https://img.shields.io/badge/Project-In%20Development-yellow)

</div>

---

## Table of Contents

- [Project Overview](#project-overview)
- [Key Features](#key-features)
- [System Architecture](#system-architecture)
- [Hardware Components](#hardware-components)
- [Wiring and Pin Configuration](#wiring-and-pin-configuration)
- [Physical Hardware Assembly](#physical-hardware-assembly)
- [Firmware and Software](#firmware-and-software)
- [Web Dashboard](#web-dashboard)
- [Kit Customization](#kit-customization)
- [Installation and Setup](#installation-and-setup)
- [Project Gallery](#project-gallery)
- [Documentation](#documentation)
- [Future Improvements](#future-improvements)
- [Contributors](#contributors)

---

## Project Overview

The TRIOE Environmental Monitoring Kit is a modular IoT-based monitoring platform built around the TRIOE ESP32-S3 microcontroller. It integrates the MQ-135 gas sensor and BME280 environmental sensor to collect environmental measurements and present them through a round LCD display and a web-based dashboard.

The system is designed to provide real-time access to environmental information, including temperature, humidity, atmospheric pressure, and gas-sensor readings.

The kit features a custom 3D-printed enclosure consisting of a main chassis and a detachable LCD head. Its modular construction allows users to modify the enclosure, replace the display assembly, and integrate additional sensors or electronic modules.

The project also aims to provide a customizable platform that can be adapted for different IoT and embedded-system applications.

## Key Features

- **Environmental Monitoring** – Collect temperature, humidity, and atmospheric pressure using the BME280.
- **Gas-Sensor Monitoring** – Obtain analog readings from the MQ-135.
- **Round LCD Interface** – Display sensor information and system status.
- **Web Dashboard** – Provide a browser-based interface for viewing monitoring data.
- **Modular Enclosure** – Separate main chassis and detachable LCD head.
- **Customizable Display** – Allow modifications to the LCD interface and display layout.
- **Expandable Hardware** – Support additional sensors and electronic modules.
- **3D-Printed Design** – Allow physical modifications to mounting points, openings, and enclosure dimensions.

## System Architecture

The system consists of four primary sections:

1. Sensor Modules
2. ESP32-S3 Microcontroller
3. Round LCD Display
4. Web Dashboard

![System Architecture](assets/diagrams/system-architecture.png)

### System Operation

1. The BME280 collects temperature, humidity, and atmospheric pressure measurements.
2. The MQ-135 produces analog sensor readings.
3. The ESP32-S3 processes the collected sensor information.
4. The LCD displays the current monitoring information.
5. The ESP32-S3 communicates with the web dashboard through a wireless connection.
6. The dashboard presents available sensor readings and system information.

## Hardware Components

| Component | Function |
|---|---|
| TRIOE ESP32-S3 | Main microcontroller |
| MQ-135 | Gas-sensing module |
| BME280 | Temperature, humidity, and pressure sensor |
| Round LCD | Local display interface |
| Main Chassis | Houses the electronic components |
| LCD Head | Holds the round display |
| Male Pin Headers | Electrical connections |
| Connecting Wires | Component interconnections |

## Wiring and Pin Configuration

The following table shows the current wiring configuration.

### MQ-135

| MQ-135 Pin | ESP32-S3 Connection | Purpose |
|---|---|---|
| VCC | 5V | Power |
| GND | GND | Ground |
| A0 | D4 | Analog Output |

### Round LCD (SPI)

| LCD Pin | ESP32-S3 Connection | Purpose |
|---|---|---|
| VCC | 3.3V | Display Power |
| GND | GND | Ground |
| SCL | D14 | SPI Clock |
| SDA | D10 | SPI MOSI |
| DC | D9 | Data/Command |
| CS | D8 | Chip Select |
| RST | D7| Reset |

### BME280 (I2C)

| BME280 Pin | ESP32-S3 Connection | Purpose |
|---|---|---|
| VCC | 3.3V | Power |
| GND | GND | Ground |
| SCL | D2 | I2C Clock |
| SDA | D1 | I2C Data |

**Hardware Note:** Verify the TRIOE board's D-pin-to-GPIO mapping before configuring firmware. Also verify that the MQ-135 module's analog output does not exceed the ESP32-S3 ADC input voltage limit. Use appropriate voltage conditioning when necessary.

Refer to [hardware/wiring/pinout-guide.md](hardware/wiring/pinout-guide.md) for the complete wiring reference.

## Physical Hardware Assembly

The enclosure consists of a main chassis and a separate LCD head.

### Assembly Procedure

1. Prepare the TRIOE ESP32-S3, MQ-135, BME280, LCD, and enclosure components.
2. Install the required pin headers on the electronic modules.
3. Insert the MQ-135 into the large square opening.
4. Position the BME280 in the smaller opening near the upper portion of the chassis.
5. Mount the TRIOE ESP32-S3 using the three designated mounting posts.
6. Install the LCD onto the separate 3D-printed head.
7. Attach the LCD head using the two mounting points near the BME280.
8. Connect the sensors and LCD according to the wiring diagram.
9. Inspect the assembly and ensure that the components and wires are properly positioned.
10. Power the system and verify the operation of the display and sensors.

### Assembly Reference

![Hardware Assembly](assets/product-images/hardware-assembly.png)

## Firmware and Software

The firmware runs on the TRIOE ESP32-S3 and handles sensor acquisition, data processing, display updates, and wireless communication.

### Software Technologies

- Arduino IDE
- C/C++
- ESP32 Arduino Core
- BME280 sensor library
- MQ-135 sensor processing
- SPI display communication
- I2C communication
- Wi-Fi connectivity

### Firmware Modules

| Module | Responsibility |
|---|---|
| Main Program | Initializes and coordinates system operations |
| BME280 Driver | Reads environmental measurements |
| MQ-135 Driver | Reads and processes gas-sensor data |
| Display Module | Updates the round LCD interface |
| Wi-Fi Module | Manages wireless connectivity |
| Sensor Processing | Handles filtering and measurement calculations |

## Web Dashboard

The web dashboard provides an additional interface for viewing the monitoring system's available data.

### Planned Dashboard Features

- Real-time temperature readings
- Humidity monitoring
- Atmospheric pressure monitoring
- MQ-135 sensor readings
- Air-quality indicator
- Device connection status
- Historical sensor data
- Customizable LCD display interface

![Dashboard Preview](assets/screenshots/dashboard-preview.png)

Dashboard functionality will depend on the implemented firmware and communication protocol.

## Kit Customization

The kit is designed with a modular structure that allows users to modify and expand its capabilities.

### 1. Air Quality Monitoring

Use the MQ-135 to obtain gas-related sensor readings and monitor changes in the surrounding environment.

### 2. Environmental Monitoring

Use the BME280 to measure temperature, humidity, and atmospheric pressure.

### 3. LCD Display Customization

Modify the display interface to show different information, including:

- Temperature
- Humidity
- Pressure
- Gas-sensor readings
- Air-quality indicators
- System warnings

### 4. Microcontroller Mounting

The three mounting points secure the TRIOE ESP32-S3 inside the main chassis while maintaining access to its connections.

### 5. Modular Sensor Expansion

Additional sensors can be integrated by modifying the enclosure and connecting compatible modules.

Possible additions include:

- Light sensors
- Motion sensors
- Distance sensors
- Additional gas sensors
- Other environmental sensors

### 6. Customizable Enclosure

The 3D-printed chassis can be modified to accommodate different components, mounting positions, cable openings, and display configurations.

### 7. Replaceable LCD Head

The detachable LCD head allows alternative display sizes, orientations, and interface modules to be incorporated.

## Installation and Setup

### Requirements

- TRIOE ESP32-S3
- Arduino IDE
- USB data cable
- Required sensor and display libraries
- Wi-Fi connection for dashboard functionality

### Firmware Installation

1. Clone this repository:

   ```bash
   git clone https://github.com/USERNAME/trioe-environmental-monitoring-kit.git
   ```

2. Open the firmware project in Arduino IDE.
3. Install the required libraries.
4. Select the appropriate ESP32-S3 board configuration.
5. Verify the GPIO assignments.
6. Configure the wireless network credentials if required.
7. Connect the TRIOE ESP32-S3.
8. Compile and upload the firmware.
9. Verify the sensor readings and LCD operation.

### Hardware Installation

Follow the instructions in:

[Physical Hardware Assembly](hardware/assembly/hardware-assembly.md)

## Project Gallery

### Physical Hardware

![Main Chassis](assets/product-images/main-chassis.png)

![LCD Head](assets/product-images/lcd-head.png)

![Completed Device](assets/product-images/completed-device.png)

### Software Interface

![LCD Interface](assets/screenshots/lcd-interface.png)

![Web Dashboard](assets/screenshots/dashboard.png)

## Documentation

For additional technical information, refer to the following documents:

| Document | Description |
|---|---|
| System Overview | Project architecture and operating principles |
| Hardware Setup | Component connections and assembly |
| Pinout Guide | ESP32-S3 pin assignments |
| Sensor Calibration | Sensor calibration and measurement considerations |
| Dashboard Guide | Dashboard installation and usage |
| Customization Guide | Hardware and interface modifications |
| Troubleshooting | Common issues and solutions |

## Future Improvements

The project can be expanded through additional hardware and software features, such as:

- Improved sensor calibration and measurement stability.
- Customizable LCD themes and layouts.
- Additional environmental monitoring sensors.
- Historical data visualization.
- Cloud-based monitoring.
- Device configuration through the web dashboard.
- Improved enclosure design and cable management.
- Multiple-device monitoring.

## Contributors

| Name | Role |
|---|---|
| Matt Matienzo | Hardware and Firmware Development |


## License

This project is distributed under the MIT License. Refer to the LICENSE file for details.

