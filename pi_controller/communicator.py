"""
Handles serial communication with the ESP32 device.
"""
import time

# The 'pyserial' library is required to use this module.
# It can be installed with: pip install pyserial
try:
    import serial
except ImportError:
    print("---------------------------------------------------------------------")
    print("Error: The 'pyserial' library is not installed.")
    print("Please install it to enable communication with the ESP32:")
    print("    pip install pyserial")
    print("---------------------------------------------------------------------")
    # We create a dummy class to avoid crashing the main application on import
    # if pyserial is not present. The error will be caught during the function call.
    class serial:
        class Serial:
            def __init__(self, *args, **kwargs):
                raise ImportError("pyserial is not installed.")
        class SerialException(Exception):
            pass


def send_to_esp32(json_payload: str, port: str, baud: int, timeout: int = 5) -> bool:
    """
    Sends the formatted JSON payload to the ESP32 over a serial connection.

    Args:
        json_payload: The JSON string to send to the device.
        port: The serial port where the ESP32 is connected (e.g., '/dev/ttyUSB0').
        baud: The baud rate for the serial connection (e.g., 115200).
        timeout: The time in seconds to wait for the write operation.

    Returns:
        True if the data was sent successfully, False otherwise.
    """
    if not json_payload:
        print("[!] Communicator Error: The provided data payload is empty.")
        return False

    print(f"[*] Attempting to connect to ESP32 on port '{port}' at {baud} baud.")

    try:
        # The 'with' statement ensures the serial port is automatically closed
        # even if errors occur.
        with serial.Serial(port, baud, timeout=timeout) as ser:
            # A short delay can help ensure the connection is stable before writing.
            time.sleep(2)

            # The ESP32 firmware reads from the UART buffer until a newline
            # character is encountered. This is a critical part of the protocol.
            payload_with_newline = json_payload + '\\n'

            # The string must be encoded into bytes for serial transmission.
            data_to_send = payload_with_newline.encode('utf-8')

            print(f"[*] Writing {len(data_to_send)} bytes to the serial port...")
            ser.write(data_to_send)
            print("[+] Data sent successfully.")
            return True

    except ImportError:
        # This error is caught if the dummy serial class was used.
        # The initial error message was already printed at import time.
        return False
    except serial.SerialException as e:
        print(f"\\n[!!!] SERIAL COMMUNICATION FAILED [!!!]")
        print(f"      Error: {e}")
        print(f"      Could not open or write to the port '{port}'.")
        print( "      Please check the following:")
        print( "      1. Is the ESP32 device plugged into the Raspberry Pi?")
        print( "      2. Is the correct serial port specified? Try running 'ls /dev/tty*' to find it.")
        print( "      3. Do you have the correct permissions? You may need to run as root or add your user to the 'dialout' group.")
        return False
    except Exception as e:
        print(f"\\n[!!!] An unexpected error occurred during serial communication: {e}")
        return False
