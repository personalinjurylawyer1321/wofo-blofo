"""
Handles the transformation of Wigle API data into the format required by the ESP32.
"""
import json

def format_for_esp32(wigle_data: dict) -> str:
    """
    Formats raw data from the Wigle API into the JSON format for the ESP32.

    The ESP32 firmware expects a JSON object with a single key, "results",
    which contains a list of network objects. Each object must have specific keys:
    'type', 'ssid', 'netid', 'channel', and 'rssi'.

    Args:
        wigle_data: A dictionary representing the parsed JSON response from the
                    Wigle API (or the simulated equivalent).

    Returns:
        A JSON formatted string ready to be sent to the ESP32.
        Returns None if the input data is invalid.
    """
    if not wigle_data or 'results' not in wigle_data:
        print("[!] Formatter Error: Invalid or empty data received.")
        return None

    processed_networks = []
    for network in wigle_data['results']:
        # Skip records that are missing essential information
        if not all(k in network for k in ['netid', 'type', 'rssi']):
            print(f"[!] Formatter Warning: Skipping malformed network record: {network}")
            continue

        # Handle hidden networks (empty SSID) by providing a placeholder.
        # The ESP32 code expects a non-empty string to broadcast.
        ssid = network.get('ssid', '').strip()
        if not ssid:
            ssid = "<hidden>"

        # The ESP32's WiFi code uses the channel. For BLE devices, Wigle may not
        # provide a channel, so we default to a common BLE advertising channel (37).
        channel = network.get('channel', 37 if network.get('type') == 'BLE' else 1)

        formatted_network = {
            "type": network.get('type', 'WIFI').upper(),
            "ssid": ssid,
            "netid": network.get('netid', '00:00:00:00:00:00'),
            "channel": channel,
            "rssi": network.get('rssi', -99)
        }
        processed_networks.append(formatted_network)

    # The final payload must be a dictionary with the "results" key.
    final_payload = {"results": processed_networks}

    # Return the payload as a compact JSON string.
    return json.dumps(final_payload)
