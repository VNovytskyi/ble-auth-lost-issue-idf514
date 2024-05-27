# ble-auth-lost-issue-idf514
Test project to reproduce the issue related to BLE client authentication when switching from version ESP-IDF 5.1.3 to version ESP-IDF 5.1.4
## Components
- ESP32 development board (rev 0)
- nRF Connect application
- installed ESP-IDF 5.1.3 and ESP-IDF 5.1.4
## Project base
Server example of ESP-NIMBLE-CPP (https://github.com/h2zero/esp-nimble-cpp)
## Steps to reproduce the issue
1) Clear bond with your ESP32 device
2) Build and flash the project with ESP-IDF 5.1.3
3) Bond with your ESP32 device (you can use nRF Connect application)
4) Reconnect
   
Now you must see:
```
--- onAuthenticationComplete ---
Authenticated: Yes
Encrypted:     Yes
Bonded:        Yes
```
5) Build and flash the project with ESP-IDF 5.1.4
6) Connect to ESP32

Now you must see:
```
--- onAuthenticationComplete ---
Authenticated: No
Encrypted:     Yes
Bonded:        Yes
```
## Expected result
The "Authenticated" flag after an update is "Yes"
## Actual result
The "Authenticated" flag after an update is "No"
