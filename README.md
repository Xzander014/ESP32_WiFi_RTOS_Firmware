# ESP32_WiFi_RTOS_Firmware

Dual-core ESP32 firmware built with **ESP-IDF + FreeRTOS**, demonstrating task separation across cores, HTTP-based command handling, inter-task communication, and persistent configuration storage using NVS.

The project separates **networking, command processing, and logging** into independent FreeRTOS tasks for clarity, scalability, and robustness.

---

## Features

* Wi-Fi station mode with auto-reconnect
* HTTP server for remote command control
* FreeRTOS queues for inter-task communication
* Mutex-protected logging
* Persistent configuration using NVS (boot counter)
* Tasks pinned to separate ESP32 cores

---

## Prerequisites

* ESP32 development board
* Linux / macOS / Windows
* USB cable
* Python 3

---

## Installing ESP-IDF

Follow the **official ESP-IDF installation guide** (recommended):

👉 [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/)

After installation, make sure the environment is loaded:

```bash
. $HOME/esp/esp-idf/export.sh
```

Verify:

```bash
idf.py --version
```

---

## Project Structure

```text
ESP32_WiFi_RTOS_Firmware/
├── main/
│   └── main.c          # Application logic (Wi-Fi, HTTP, RTOS tasks)
├── CMakeLists.txt      # Project build configuration
├── sdkconfig           # ESP-IDF project configuration
├── .gitignore
└── README.md
```

### Key Components in `main.c`

* **Wi-Fi Manager**

  * Initializes STA mode
  * Handles reconnects via event handler

* **HTTP Server**

  * `/cmd?cmd=<id>` endpoint
  * Sends commands to processing task via queue

* **Command Processor Task (Core 1)**

  * Executes commands like LED control, status, reboot

* **Logger Task (Core 0)**

  * Periodic system state + heap logging

* **NVS Storage**

  * Stores persistent boot counter across resets

---

## Configuration

Edit Wi-Fi credentials in `main.c`:

```c
#define WIFI_SSID "Your_SSID"
#define WIFI_PASS "Your_Password"
```

---

## Building the Project

From the project root:

```bash
idf.py set-target esp32
idf.py build
```

---

## Flashing & Monitoring

```bash
idf.py flash monitor
```

Exit monitor:

```
Ctrl + ]
```

---

## HTTP Command Interface

Once connected to Wi-Fi, send commands via browser or curl:

```text
http://<ESP32_IP>/cmd?cmd=<ID>
```

### Supported Commands

| Command ID | Action       |
| ---------: | ------------ |
|          1 | LED ON       |
|          2 | LED OFF      |
|          3 | Get status   |
|          4 | Reboot ESP32 |

Example:

```bash
curl "http://192.168.1.100/cmd?cmd=3"
```

---

## Core Assignment

| Task              | Core |
| ----------------- | ---- |
| Command Processor | 1    |
| Logger            | 0    |

This separation prevents networking and logging from blocking command execution.

---

## Notes

* `build/` directory is auto-generated and intentionally ignored
* Credentials are hardcoded for demo purposes (move to NVS for production)
* LED control is currently stubbed with logs

---

## Future Improvements

* HTTPS support
* JSON-based commands
* Dynamic Wi-Fi provisioning
* OTA updates

---

## License

MIT (or update as needed)

---


