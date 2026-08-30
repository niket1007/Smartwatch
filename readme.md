ESP32-S3 ESP-IDF Smartwatch Project

**GadgetBridge Ble Details**
1) https://www.espruino.com/Gadgetbridge

2) https://codeberg.org/Freeyourgadget/Gadgetbridge/src/branch/master/app/src/main/java/nodomain/freeyourgadget/gadgetbridge/service/devices/banglejs/BangleJSDeviceSupport.java

3) Bangle.js device support file: app\src\main\java\nodomain\freeyourgadget\gadgetbridge\service\devices\banglejs\BangleJSDeviceSupport.java

4) Flow for command sent from watch to gadgetBridge
```
onCharacteristicChanged -> handleUartRxLine -> handleUartRxJSON -> as per key "t" provided in payload
```
