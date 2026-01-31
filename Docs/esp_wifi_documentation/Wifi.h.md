**Wi-Fi SSID** = **Service Set Identifier**

It is simply the **name of a Wi-Fi network**.

---

### What it actually is

- The SSID is the **identifier broadcast by a Wi-Fi router/access point**
    
- This is the name you see when you open Wi-Fi settings on your phone or laptop
    

Examples: home_network

---

### In simple terms

- **Wi-Fi = the wireless technology**
    
- **SSID = the network’s name**
    

---

### In embedded / programming context

Example (ESP32 / ESP8266):

`const char* ssid = "MyHomeWiFi"; const char* password = "mypassword";`

- `ssid` → Wi-Fi network name
    
- `password` → network key
    

If the SSID is wrong, the device **will not connect**, even if the password is correct.

![[Pasted image 20260125172323.png]]



*setup for wifi in esp*
esp_wifi_init():
	configures the wifi hardwares including drivers and allocated all the resources required for the operation
esp_netif_init():
	initalises the network interface parameters.
esp_event_loop_create_default():
	creates an event in RTOS in the background since wifi and IP is event driven
	has connection status, disconnects and IP accusition
esp_netif_create_default_wifi_sta():
	connects to wifi and  data transfer because it defines how the packets are going to be send 
	puts the esp in a station mode
wifi_init_config_default():
	- Buffer sizes ,Task priorities ,Stack sizes, Feature flags
	- is called in wifi_init_config_t data type and fills it up with the requierd values
	- usually contains the ssid and the password for the required wifi to connect
Correct Order
--nvs_flash_init()
--esp_netif_init()
--esp_event_loop_create_default()
--esp_wifi_init()
