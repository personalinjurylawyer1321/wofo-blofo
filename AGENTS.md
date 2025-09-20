# AGENT INSTRUCTIONS

This document provides a technical overview of the Network Simulator project for AI agents and developers.

## Project Architecture

This project is composed of two main, independent components that work together:

1.  **The ESP32 Firmware (The Broadcaster):** Located in the root directory, this C code is designed to run on an ESP32 microcontroller. Its only job is to listen for commands and broadcast Wi-Fi and BLE networks accordingly.
2.  **The Raspberry Pi Controller (The Brains):** Located in the `pi_controller/` directory, this Python application is the user-facing controller. It determines *what* to broadcast, and tells the ESP32 to do it.

---

### 1. The ESP32 Firmware

-   **Primary Source File:** `main/main.c`
-   **Framework:** ESP-IDF
-   **Purpose:** To act as a remote-controlled network broadcaster.
-   **Communication Protocol:**
    -   It listens on a **UART (serial) port at 115200 baud**.
    -   It buffers incoming serial data until it receives a **newline character (`\n`)**.
    -   It parses the received data as a **JSON string**.
-   **Required JSON Structure:**
    The firmware expects a JSON object with a single top-level key: `"results"`. The value of this key must be an array of network objects. Each object in the array must have the following keys:
    ```json
    {
      "type": "WIFI" or "BLE",
      "ssid": "The Network Name",
      "netid": "The MAC Address (e.g., 0A:1B:2C:3D:4E:5F)",
      "channel": 11,
      "rssi": -80
    }
    ```

### 2. The Raspberry Pi Controller

-   **Location:** All code is contained within the `pi_controller/` directory.
-   **Purpose:** To provide a user interface for collecting parameters, fetching network data, formatting it, and sending it to the ESP32.
-   **Key Modules:**
    -   `main.py`: The main entry point for the user. Handles command-line argument parsing and calls the other modules in sequence.
    -   `wigle.py`: **This is a simulation module.** It mimics a call to the Wigle.net API and returns hardcoded sample data. To use real-world data, this is the file to modify.
    -   `formatter.py`: The critical link between the two components. It takes the (simulated) Wigle data and transforms it into the exact JSON structure required by the ESP32 firmware.
    -   `communicator.py`: Handles the physical communication. It connects to the serial port, sends the JSON payload (with the required `\n` terminator), and handles errors. It requires the `pyserial` library.
-   **User Documentation:** The `pi_controller/README.md` file contains instructions for end-users on how to set up and run the controller application.

### Development Workflow

-   To modify broadcasting behavior (e.g., beacon frame content), edit the C code in `main/main.c`.
-   To modify the data source or user interface, edit the Python files in `pi_controller/`.
-   The Python application can be tested without hardware, as the API call and serial communication are abstracted into their own modules.
-   A full end-to-end test involves running `pi_controller/main.py` on a Raspberry Pi that is connected to a flashed ESP32 via USB.
