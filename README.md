file directory:
spoofing_project/
│
├── web_interface/          # Google Maps interface
│   ├── static/
│   │   ├── css/
│   │   │   └── style.css
│   │   ├── js/
│   │   │   ├── map.js
│   │   │   └── main.js
│   │   └── images/
│   ├── templates/
│   │   └── index.html
│   ├── app.py               # Flask backend
│   ├── config.py            # API keys, paths
│   └── requirements.txt
│
├── scripts/
│   ├── fetch_wigle_data.py  # Fetch data from Wigle.net
│   ├── generate_configs.py # Generate ESP32 configs
│   └── start_services.sh   # Start all services
│
├── data/
│   ├── wifi/
│   ├── cell/
│   └── bluetooth/
│
├── esp32/
│   ├── esp32_2432s028r/      # ESP32-2432S028R (touchscreen)
│   │   ├── main/
│   │   │   └── main.c       # ESP-IDF code
│   │   └── build/
│   ├── esp32_devkit1/        # ESP32 DevKit1
│   │   └── wifi_spoofer/
│   └── esp8266/              # ESP8266
│       └── bluetooth_spoofer/
│
├── mitm_proxy/
│   ├── mitm_script.py
│   ├── cell_data/
│   └── logs/
│
├── logs/
│   ├── interface.log
│   ├── esp32.log
│   └── proxy.log
│
└── README.md
