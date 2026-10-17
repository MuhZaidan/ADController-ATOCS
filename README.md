# ADC — Analog to Digital Controller

### A Modular Plug & Play Sensor Controller for Industrial Automation

**ADC (Analog to Digital Controller)** is an ESP32-based modular controller
designed to extend the capabilities of PLC systems that lack sufficient analog
input support.

Inspired by the flexibility of smart relays such as Zelio Logic, this project
allows users to configure control logic directly within the module, monitor
sensor values, and control relay outputs without relying entirely on the host
PLC for sensor processing.

The project aims to provide a flexible and accessible approach to integrating
different sensor types into existing automation systems through a modular Plug &
Play architecture.

> **Project Status:** Prototype in Development

---

## Table of Contents

- [Project Overview](#project-overview)
- [Background and Motivation](#background-and-motivation)
- [Key Features](#key-features)
- [System Architecture](#system-architecture)
- [Hardware Components](#hardware-components)
- [Supported Sensors](#supported-sensors)
- [How It Works](#how-it-works)
- [Technology Stack](#technology-stack)
- [Repository Structure](#repository-structure)
- [Getting Started](#getting-started)
- [Current Limitations](#current-limitations)
- [Roadmap](#roadmap)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)

---

## Project Overview

In conventional industrial automation systems, sensor signals are typically
processed by a PLC or a dedicated signal-conditioning module. However, not every
PLC available in educational laboratories or industrial environments provides
analog input capabilities.

ADC addresses this limitation by introducing a separate controller that can read
sensor data, execute user-defined logic, and operate relay outputs locally.

The system consists of two primary components:

- **Controller Module** — The main control unit responsible for sensor
  monitoring, logic execution, user interaction, and relay control.
- **Sensor Module** — A modular sensor interface containing sensor-specific
  hardware and an EEPROM for sensor identification.

The intended Plug & Play concept allows the controller to identify the connected
sensor module and adapt its operation according to the stored sensor identity.

### Project Objectives

- Extend analog sensor integration capabilities for PLC systems.
- Enable local control logic configuration without requiring all logic to reside
  in the PLC.
- Provide real-time monitoring of sensor values and relay states.
- Support a modular architecture for different sensor types.
- Develop a practical, cost-effective controller for industrial automation
  education and prototyping.

## Background and Motivation

Many automation projects require analog measurements, such as temperature,
distance, or variable resistance. When a PLC does not provide suitable analog
inputs, additional hardware or signal conversion may be required.

This project explores an alternative approach: processing sensor information in
a dedicated controller before using its relay outputs within a larger automation
system.

Rather than functioning only as a signal converter, ADC is designed to combine
sensor interfacing, local logic execution, monitoring, and output control in one
module.

The long-term goal is to make sensor integration more modular, reusable, and
easier to configure.

## Key Features

| Feature                  | Description                                                                        |
| ------------------------ | ---------------------------------------------------------------------------------- |
| Sensor Identification    | Identifies the connected sensor module using identification data stored in EEPROM. |
| Sensor Monitoring        | Displays the actual value received from the connected sensor.                      |
| Relay Monitoring         | Displays the current state of the relay outputs.                                   |
| Local Logic Control      | Allows control logic to be configured and executed directly within the controller. |
| Modular Sensor Interface | Uses a 10-pin socket to connect the controller and sensor module.                  |
| User Navigation          | Uses push buttons to navigate the controller interface.                            |
| Multi-Sensor Concept     | Designed to accommodate different sensor types through modular sensor hardware.    |

_Feature availability may vary as the prototype continues to develop._

## System Architecture

The system is divided into a controller and interchangeable sensor modules.

```text
┌─────────────────────────────┐
│       SENSOR MODULE         │
│                             │
│  Sensor                     │
│    │                        │
│    ├── Sensor Signal        │
│    │                        │
│  AT24C256 EEPROM            │
│    │                        │
│    └── Sensor Identification│
└──────────────┬──────────────┘
               │
         10-Pin Socket
               │
┌──────────────▼──────────────┐
│      CONTROLLER MODULE      │
│                             │
│          ESP32              │
│            │                │
│    ┌───────┼────────┐       │
│    ▼       ▼        ▼       │
│ Sensor   Control   User     │
│ Reading   Logic   Interface │
│    │       │                │
│    └───────┼────────────────│
│            ▼                │
│      4-Channel Relay        │
│                             │
│       LCD 20x4 I2C          │
└────────────┬────────────────┘
             │
       Relay Outputs
             │
             ▼
    External Control System
```

_Conceptual architecture. Electrical connections, communication details, and the
final pin assignment are subject to verification against the implemented
hardware._

## Hardware Components

### 1. Controller Module

| Component              | Function                                                                                        |
| ---------------------- | ----------------------------------------------------------------------------------------------- |
| ESP32 DevKit           | Main controller for sensor processing and logic execution.                                      |
| 20×4 I2C LCD           | Displays sensor values, relay states, and controller interface information.                     |
| 5V Step-Down Converter | Provides a regulated 5V supply from a higher-voltage source, subject to the final power design. |
| 4-Channel Relay Module | Provides four relay-controlled output channels.                                                 |
| 10-Pin Socket          | Interface for connecting the sensor module.                                                     |
| Potentiometer          | Adjustable analog input for testing and control applications.                                   |
| Push Buttons           | Navigation and user interaction.                                                                |

### 2. Sensor Module

| Component       | Function                                                     |
| --------------- | ------------------------------------------------------------ |
| Sensor          | Provides measurements according to the selected sensor type. |
| AT24C256 EEPROM | Stores sensor identification data.                           |
| 10-Pin Socket   | Provides the physical interface to the controller module.    |

The modular design separates sensor-specific hardware from the main controller,
making it easier to develop and test additional sensor modules independently.

## Supported Sensors

The following sensors and input devices are included in the project's current
development scope.

| Sensor / Input             | Measurement or Input Type | Status            |
| -------------------------- | ------------------------- | ----------------- |
| HYSRF-05 Ultrasonic Sensor | Distance measurement      | Prototype support |
| DS18B20                    | Temperature measurement   | Prototype support |
| Potentiometer              | Variable analog input     | Prototype support |

_The implementation status, wiring requirements, supported measurement ranges,
and calibration details should be confirmed for each sensor before deployment._

## How It Works

The intended operating sequence is as follows:

1. **Module Connection** — Connect the sensor module to the controller through
   the 10-pin interface.
2. **Sensor Identification** — Read the sensor identification data stored in the
   module's AT24C256 EEPROM.
3. **Sensor Initialization** — Select the appropriate sensor handling routine
   based on the identified sensor type.
4. **Data Acquisition** — Read the sensor's measurement or input value.
5. **Logic Evaluation** — Evaluate the acquired value against the control logic
   configured within the controller.
6. **Output Control** — Activate or deactivate the appropriate relay outputs
   according to the logic result.
7. **Monitoring** — Display the actual sensor value and relay states on the 20×4
   LCD.

The exact initialization procedure, logic configuration method, and EEPROM data
format will be documented as the firmware implementation is finalized.

## Technology Stack

| Technology              | Purpose                                                                      |
| ----------------------- | ---------------------------------------------------------------------------- |
| C++                     | Controller firmware development.                                             |
| ESP32                   | Embedded processing platform.                                                |
| I2C                     | Interface for the LCD and EEPROM, where supported by the implemented wiring. |
| AT24C256 EEPROM         | Non-volatile storage for sensor identification data.                         |
| Relay Interface         | Switching control outputs.                                                   |
| Modular Hardware Design | Separation of controller and sensor-specific modules.                        |

## Repository Structure

The following is a proposed documentation structure. Replace the placeholders
with the actual directories and filenames used in this repository.

```text
Modul-Plug-and-Play/
├── README.md
├── firmware/
│   ├── controller/
│   └── sensor_modules/
├── hardware/
│   ├── controller/
│   └── sensor_modules/
├── docs/
│   ├── system_architecture.md
│   ├── wiring_diagram.md
│   └── user_guide.md
├── images/
│   ├── controller_prototype.jpg
│   └── sensor_module.jpg
└── LICENSE
```

> **Note:** This structure is illustrative. Update it to match the actual
> repository before publishing.

## Getting Started

### Prerequisites

Before setting up the project, prepare the following:

- ESP32 DevKit.
- Controller and sensor module hardware.
- A compatible USB data cable.
- Arduino IDE or another compatible ESP32 development environment.
- The libraries required by the firmware.

The exact ESP32 board package version, library dependencies, and firmware upload
procedure have not yet been specified.

### Setup Procedure

1. Clone or download this repository.
2. Open the controller firmware project in the selected development environment.
3. Install the required libraries listed in the firmware documentation.
4. Verify the board selection, GPIO configuration, and communication settings.
5. Connect the controller hardware according to the verified wiring diagram.
6. Upload the firmware to the ESP32.
7. Connect a supported sensor module and verify sensor identification.
8. Check the displayed sensor value and relay states.

**Important:** Do not connect industrial voltages or external loads until the
power supply, grounding, relay ratings, isolation, and connector pinout have
been verified.

## Current Limitations

This project is currently a prototype. The following details require further
implementation or documentation:

- Final 10-pin connector assignment and electrical specifications.
- Complete sensor compatibility and initialization details.
- User interface workflow for configuring control logic.
- Sensor calibration and measurement range documentation.
- Power distribution and electrical protection specifications.
- Firmware installation instructions and verified library dependencies.
- Tested integration procedure with different PLC models.

These limitations will be updated as the hardware and firmware mature.

## Roadmap

The following items represent potential development milestones and are not
claims of completed functionality.

- [x] Establish the controller and sensor-module concept.
- [x] Define the initial hardware architecture.
- [ ] Finalize the 10-pin connector specification.
- [ ] Complete and test EEPROM-based sensor identification.
- [ ] Implement sensor initialization and measurement routines.
- [ ] Develop the local logic configuration interface.
- [ ] Validate relay control and monitoring.
- [ ] Test compatibility across supported sensor modules.
- [ ] Document PLC integration and electrical interfaces.
- [ ] Prepare complete hardware diagrams and assembly instructions.

## Documentation

Additional technical documentation can be added here as it becomes available.

- **Wiring Diagram:** [Add link]
- **Controller Schematic:** [Add link]
- **Sensor Module Design:** [Add link]
- **Firmware Documentation:** [Add link]
- **Demonstration Video:** [Add link]
- **Project Images:** [Add link]

## Contributing

Contributions, technical discussions, and suggestions are welcome.

If you would like to contribute:

1. Review the existing documentation and project structure.
2. Identify an issue or improvement.
3. Propose the change with a clear technical explanation.
4. Test hardware or firmware changes before submitting them.

Please document any changes that affect the electrical interface, sensor
identification format, or controller behavior to maintain compatibility across
modules.

## License

The project's license has not yet been specified.

Add a `LICENSE` file and update this section once the licensing terms have been
decided.

---

**ADC — Analog to Digital Controller**

_Exploring a more modular approach to sensor integration and local control logic
in industrial automation._

**Project Status:** Prototype in Development  
**Primary Platform:** ESP32  
**Programming Language:** C++  
**Application Domain:** Industrial Automation and Embedded Systems
