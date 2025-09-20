"""
Simulates interaction with the Wigle.net API for development and testing.
"""

def fetch_data(location: str, radius: int):
    """
    Simulates fetching data from the Wigle.net API.

    In a real-world application, this function would use a library like `requests`
    to make an authenticated GET request to the Wigle API's /network/search
    endpoint and return the parsed JSON response.

    Args:
        location (str): The location string (ignored in this simulation).
        radius (int): The radius in meters (ignored in this simulation).

    Returns:
        A dictionary containing simulated network data structured like a real
        Wigle API response. Returns None if the simulation "fails".
    """
    print(f"[*] (Simulation) Searching for networks near '{location}' within a {radius}m radius.")

    # This is a sample response modeled after the Wigle API's structure.
    # It contains a mix of WiFi and Bluetooth (BLE) networks.
    sample_data = {
        "success": True,
        "totalResults": 8,
        "first": 0,
        "last": 8,
        "resultCount": 8,
        "results": [
            {
                "trilat": 40.7128, "trilong": -74.0060, "ssid": "Starbucks Free WiFi",
                "netid": "0A:1B:2C:3D:4E:5F", "type": "WIFI", "channel": 11, "rssi": -65, "wep": "f"
            },
            {
                "trilat": 40.7129, "trilong": -74.0061, "ssid": "xfinitywifi",
                "netid": "1A:2B:3C:4D:5E:6F", "type": "WIFI", "channel": 6, "rssi": -72, "wep": "f"
            },
            {
                "trilat": 40.7130, "trilong": -74.0062, "ssid": "MySecureNetwork",
                "netid": "A1:B2:C3:D4:E5:F6", "type": "WIFI", "channel": 1, "rssi": -50, "wep": "2"
            },
            {
                "trilat": 40.7131, "trilong": -74.0063, "ssid": "Another AP",
                "netid": "AA:BB:CC:DD:EE:FF", "type": "WIFI", "channel": 8, "rssi": -85, "wep": "1"
            },
            {
                "trilat": 40.7127, "trilong": -74.0059, "ssid": "JBL Go 2",
                "netid": "F0:E1:D2:C3:B4:A5", "type": "BLE", "rssi": -88, "channel": 37
            },
            {
                "trilat": 40.7126, "trilong": -74.0058, "ssid": "LE-Bose QC35 II",
                "netid": "00:11:22:33:44:55", "type": "BLE", "rssi": -75, "channel": 38
            },
            {
                "trilat": 40.7125, "trilong": -74.0057, "ssid": "Fitbit Charge 3",
                "netid": "DE:AD:BE:EF:CA:FE", "type": "BLE", "rssi": -90, "channel": 39
            },
            {
                "trilat": 40.7132, "trilong": -74.0064, "ssid": "", # Hidden network
                "netid": "11:22:33:44:55:66", "type": "WIFI", "channel": 4, "rssi": -80, "wep": "2"
            }
        ]
    }
    print(f"[+] (Simulation) Found {len(sample_data['results'])} networks.")
    return sample_data
