# Network Simulator Controller (Raspberry Pi)

This Python application is the brains of the network simulator project. It runs on a Raspberry Pi and commands the ESP32 firmware to broadcast a specific set of Wi-Fi and Bluetooth networks.

## How It Works

The controller follows a simple, three-step process:

1.  **Collects Input:** It takes a target location and a search radius from the user via the command line.
2.  **Fetches Network Data:** It queries the Wigle.net API for all known networks within the specified area. (Note: The current version uses a *simulated* API call with sample data for testing).
3.  **Commands the ESP32:** It formats the collected network data into a specific JSON structure and sends it to the ESP32 over a serial (USB) connection.

Once the ESP32 receives this payload, its firmware takes over and begins broadcasting the fake networks.

## Requirements

*   A Raspberry Pi (or any computer with Python 3).
*   Python 3.
*   The `pyserial` library for serial communication.
*   An ESP32 device flashed with the firmware located in the project's root directory.

## Setup & Installation

1.  **Connect the ESP32:** Plug your flashed ESP32 device into one of the Raspberry Pi's USB ports.

2.  **Install Dependencies:** Open a terminal on your Raspberry Pi and install the `pyserial` library:
    ```bash
    pip3 install pyserial
    ```

3.  **Identify the Serial Port:** You must find the name of the serial port that the ESP32 is connected to. It is typically `/dev/ttyUSB0` or `/dev/ttyACM0`. You can find the correct port by running `ls /dev/tty*` in the terminal both *before* and *after* plugging in the ESP32. The new entry that appears is the correct port name.

## Usage

Navigate to this directory (`pi_controller`) in your terminal to run the application.

### Command Syntax
```bash
python3 main.py <location> <radius> [--port <serial_port>]
```

### Arguments
*   `<location>`: The target location. Can be a street address or GPS coordinates in the format `"latitude,longitude"`.
*   `<radius>`: The search radius in meters.
*   `--port` (optional): The serial port your ESP32 is connected to. Defaults to `/dev/ttyUSB0`.

### Examples

**Using GPS Coordinates:**
```bash
python3 main.py "40.647053,-73.957954" 500
```

**Using a Street Address:**
*(Note: The current `wigle.py` is a simulation. A real implementation would require a geocoding service to convert this address to coordinates before querying Wigle.)*
```bash
python3 main.py "999 Flatbush Ave, Brooklyn, NY 11226" 250
```

**Specifying a different serial port:**
```bash
python3 main.py "51.5074,-0.1278" 1000 --port /dev/ttyACM0
```
