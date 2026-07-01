Here is the updated, comprehensive **Power Management Plan** incorporating all the hardware buttons, low-level display routines, touch handling, timeout rules, gesture restrictions, and Light Sleep states that we have implemented in the firmware codebase.

---

### ## Smartwatch Power Management Plan

#### 1. Physical Power / Wake Button (BOOT Button - GPIO0)

* **Behavior**: Operates like a smartphone lock screen button.
* **Action**: Clicking the BOOT button manually toggles the AMOLED display between ON (`0x29` / Restored Brightness) and OFF (`0x28` / Backlight `0x00`), independent of timers or touch inactivity.
* **Debouncing**: ISR handling incorporates a 200ms software debounce lock to prevent accidental double triggering.

#### 2. Battery Level: 70% – 100%

* **Screen Timeout**: 15 seconds
* **Brightness Tier**: 70% (`0xB2`)
* **Retrieval Intervals**:
* Battery UI Update: 5000ms
* Datetime UI Update: 2000ms
* Battery Sensor Read: 2000ms


* **Gesture Navigation**: Fully active.
* **Connectivity/Network**: BLE active (when implemented); HTTP/Wi-Fi calls run every 30 minutes.

#### 3. Battery Level: 40% – 70%

* **Screen Timeout**: 10 seconds
* **Brightness Tier**: 50% (`0x7F`)
* **Retrieval Intervals**: All background intervals are doubled (slower polling to allow CPU idle yields).
* Battery UI Update: 10000ms
* Datetime UI Update: 4000ms
* Battery Sensor Read: 4000ms


* **Gesture Navigation**: Fully active.
* **Connectivity/Network**: BLE active; HTTP/Wi-Fi calls run every 60 minutes.

#### 4. Battery Level: 20% – 40%

* **Screen Timeout**: 5 seconds
* **Brightness Tier**: 40% (`0x66`)
* **Retrieval Intervals**:
* Battery UI Update: 15000ms
* Datetime UI Update: 6000ms
* Battery Sensor Read: 6000ms


* **Gesture Navigation**: Fully active.
* **Connectivity/Network**: All HTTP/Wi-Fi connectivity is completely disabled. **BLE remains connected/active**.

#### 5. Battery Level: < 20% (Ultra Power Saver)

* **Screen Timeout**: 5 seconds
* **Brightness Tier**: 15% (`0x26`) — Aggressively dimmed for the AMOLED panel.
* **Retrieval Intervals**:
* Battery UI Update: 15000ms
* Datetime UI Update: 6000ms
* Battery Sensor Read: 6000ms


* **Gesture Navigation**: **Disabled** (swipes will not trigger screen transitions; CPU stays locked to the home screen).
* **Connectivity/Network**: Both HTTP (Wi-Fi) and BLE connectivity are completely closed/turned off.
* **Wake Method**: The screen will only turn on when the physical BOOT button is clicked.

---

### ## Hardware Low-Power Standby (Light Sleep)

* **Screen Off State**: Whenever the display turns off (due to inactivity timeout or a manual button press), the FT3168 touch controller and LVGL engine gracefully suspend operations, and the ESP32-S3 drops into **Light Sleep** (`esp_light_sleep_start()`).
* **Wake-Up Source**: `GPIO0` is primed as a wake-up source (`GPIO_INTR_LOW_LEVEL`). Pressing the physical BOOT button instantly wakes the processor from its standby state, seamlessly turning the display back on and resetting the timeout counter.