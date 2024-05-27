# ble-auth-lost-issue-idf514 (IDFGH-12905)
A test project to reproduce the issue related to BLE client authentication when switching from version ESP-IDF 5.1.3 to version ESP-IDF 5.1.4. 
The problem is that the BLE bond (procedure or data format) on 5.1.3 is different from 5.1.4 and when upgrading to a new version, the authorization flag is reset, but the others do not change.

For example:
1) You have your own IoT project that uses the BLE feature. You are using the ESP-IDF 5.1.3 for developing new releases;
2) At every new release, the customers can connect to your products without entering a password because they are bonded;
3) You are using the next flags to protect data when creating characteristics: NIMBLE_PROPERTY::READ_AUTHEN, WRITE_AUTHEN, READ_ENC, WRITE_ENC;
4) Then you start using the ESP-IDF 5.1.4 and release a new version of your product;
5) Customers report that after a firmware update they have to re-enter the password to connect via BLE and this behaviour was not present in previous versions (or it can be another situation depending on BLE client: forever re-connect loop)
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
The "Authenticated" flag after an firmware update is "Yes"
## Actual result
The "Authenticated" flag after an firmware update is "No"
## Notes
Password: 123456
