"""
Main application for the Raspberry Pi network simulator controller.
"""
import argparse
import sys
import wigle
import formatter
import communicator

def main():
    """
    Main function to run the network simulator controller.

    This function parses command-line arguments, orchestrates the process of
    fetching data from Wigle, formatting it, and sending it to the ESP32.
    """
    parser = argparse.ArgumentParser(
        description="Control the ESP32-based network simulator. Fetches network data from a given location and commands the ESP32 to broadcast it.",
        epilog="Example: python3 main.py \"40.7128,-74.0060\" 500 --port /dev/ttyUSB0"
    )
    parser.add_argument(
        "location",
        type=str,
        help="Target location, either as 'latitude,longitude' or a street address."
    )
    parser.add_argument(
        "radius",
        type=int,
        help="Radius in meters around the location to search for networks."
    )
    parser.add_argument(
        "--port",
        type=str,
        default="/dev/ttyUSB0",
        help="The serial port of the connected ESP32 device (default: /dev/ttyUSB0)."
    )
    parser.add_argument(
        "--baud",
        type=int,
        default=115200,
        help="The baud rate for the serial communication (default: 115200)."
    )

    if len(sys.argv) == 1:
        parser.print_help(sys.stderr)
        sys.exit(1)

    args = parser.parse_args()

    print("--- Network Simulator Controller ---")
    print(f"[*] Target Location: {args.location}")
    print(f"[*] Search Radius: {args.radius}m")
    print(f"[*] ESP32 Port: {args.port}")
    print("------------------------------------")

    # 1. Get data from Wigle (using a simulated module for now)
    print("\n[1/3] Fetching network data...")
    raw_data = wigle.fetch_data(args.location, args.radius)
    if not raw_data or not raw_data.get('results'):
        print("[!] Failed to fetch or parse network data. Exiting.")
        return
    print(f"[+] Successfully fetched info for {len(raw_data['results'])} networks.")

    # 2. Format data for the ESP32
    print("\n[2/3] Formatting data for ESP32...")
    esp32_json_payload = formatter.format_for_esp32(raw_data)
    if not esp32_json_payload:
         print("[!] Failed to format data for the ESP32. Exiting.")
         return
    print("[+] Data formatted successfully.")

    # 3. Send data to the ESP32
    print(f"\n[3/3] Sending data to ESP32 on {args.port}...")
    success = communicator.send_to_esp32(esp32_json_payload, args.port, args.baud)
    if success:
        print("\n[SUCCESS] Data sent to ESP32. Broadcasting should now be active.")
    else:
        print("\n[FAILURE] Failed to send data to the ESP32. Check connections and port.")

if __name__ == "__main__":
    main()
