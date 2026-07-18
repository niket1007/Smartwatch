Smartwatch



ESP32-S3 ESP-IDF Smartwatch Project


**AI Flow Builder**: https://app.flowstep.ai/

**GadgetBridge Ble Details**: 
    1) https://www.espruino.com/Gadgetbridge
    2) https://codeberg.org/Freeyourgadget/Gadgetbridge/src/branch/master/app/src/main/java/nodomain/freeyourgadget/gadgetbridge/service/devices/banglejs/BangleJSDeviceSupport.java
        2.1) Bangle.js device support file: app\src\main\java\nodomain\freeyourgadget\gadgetbridge\service\devices\banglejs\BangleJSDeviceSupport.java
            2.1.1) Flow for command sent from watch to gadgetBridge
                onCharacteristicChanged -> handleUartRxLine -> handleUartRxJSON -> as per key "t" provided in payload

**SVG TO TTF**: https://icomoon.io