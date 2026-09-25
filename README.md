# Smart Mine Safety Rover

ESP32-based mine-safety rover project with a local Wi-Fi dashboard.

## Current dashboard

- ESP32 creates the Wi-Fi network `SMART_MINE_ROVER`
- Password: `mine12345`
- Dashboard: `http://192.168.4.1`
- Live gas telemetry fields
- MQ-6 voltage
- Gas-alert state
- GPS status and coordinates
- Local/offline operation

## PlatformIO structure

```text
Mine-Safety-Rover/
├── platformio.ini
├── README.md
├── .gitignore
└── src/
    └── main.cpp
```

## Build / upload

Open this folder in VS Code with PlatformIO and run:

```text
pio run
pio run -t upload
pio device monitor -b 115200
```

## Important hardware note

The repository scaffold contains the dashboard and telemetry parser. The exact final prototype pin map and LoRa packet-handling section should remain the same as the tested hardware version. Do not change GPIO assignments without checking the wiring.

## Dashboard

After upload:

1. Power the TX ESP32.
2. Connect the laptop/phone to `SMART_MINE_ROVER`.
3. Enter password `mine12345`.
4. Open `http://192.168.4.1`.

## Telemetry packet

The dashboard parser understands packets such as:

```text
GAS:24,V:0.98,ALERT:0
```

For the final rover build, connect this parser to the existing LoRa receive path.

## GitHub

Do not commit:

- `.pio/`
- `.vscode/`
- build binaries
- personal credentials
- photos containing private information
