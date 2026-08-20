- **Generalised Bresenham's line algorithm**
  - Wikipedia link: https://en.wikipedia.org/wiki/Bresenham's_line_algorithm

  - Psuedo Code
    ```
    plotLine(x0, y0, x1, y1)
        dx = abs(x1 - x0)
        sx = x0 < x1 ? 1 : -1
        dy = -abs(y1 - y0)
        sy = y0 < y1 ? 1 : -1
        error = dx + dy

        while true
            plot(x0, y0)
            e2 = 2 * error
            if e2 >= dy
                if x0 == x1 break
                error = error + dy
                x0 = x0 + sx
            end if
            if e2 <= dx
                if y0 == y1 break
                error = error + dx
                y0 = y0 + sy
            end if
        end while
    ```

- **Midpoint Circle algorithm**
  - Link: https://medium.com/@dillihangrae/mid-point-circle-algorithm-84f5971dcd08
  - Psuedo Code

    ```
    plot_symmetric_points(cx, cy, x, y)
        draw(cx + x, cy + y)
        draw(cx - x, cy + y)
        draw(cx + x, cy - y)
        draw(cx - x, cy - y)

        draw(cx + y, cy + x)
        draw(cx - y, cy + x)
        draw(cx + y, cy - x)
        draw(cx - y, cy - x)

    plotCircle(cx, cy, r)
        x = 0
        y = r
        dp = 1 - r // decision parameter

        while x <= y
            plot_symmetric_points(cx, cy, x, y)
            x = x + 1
            if dp < 0
                dp = dp + (2 * x) + 1
            else
                y = y - 1
                dp = dp + (2 * x) - (2 * y) + 1
            end if
        end while
    ```

- **Rounded Rectangle**
  - Diagrams:

    ![alt text](image-2.png)
    ![alt text](image-1.png)

  - Logic

    ```
        1) First create 4 lines (Look for black bar in diagram)
            1.1) Top Line
                Start: x+r, y
                End: (x+width-1)-r, y
            1.2) Bottom Line
                Start: x+r, y+height-1
                End: (x+width-1)-r, (y+height-1)
            1.3) Left Line
                Start: x, y+r
                End: x, (y+height-1)-r
            1.4) Right Line
                Start: (x+width-1), y+r
                End: (x+width-1), (y+height-1)-r

        2) Draw arc for radis r in each corner (Look for red dot in diagram)
            2.1) Top Left Arc
                Center: x+r, y+r
            2.2) Bottom Left Arc
                Center: x+r, (y+height-1)-r
            2.3) Top Right Arc
                Center: (x+width-1)-r, y+r
            2.4) Bottom Right Arc
                Center: (x+width-1)-r, (y+height-1)-r
    ```

- **Icons**
  1. Go to [Figma](https://www.figma.com/design/9fLX7xC7nFDrpWQDLoFpeR/IconSet?node-id=101-3002&t=q5NevkQGi0K0dfgN-0) and download svg file
  2. Place the svg file inside Graphics/Icons/svg
  3. Run icon_conveter.py file in converter folder

- **Fonts**
  1. Download ttf and place it inside Graphics/Fonts/ttf_and_c
  2. Go to [lvgl font converter](https://lvgl.io/tools/fontconverter)

     2.1) Upload the ttf file

     2.2) Provide file name

     2.3) Select bits per pixel as 4

  3. Place the dowloaded c file inside Graphics/Fonts/ttf_and_c and change extension from .c to .txt
  4. Run font_extractor.py file in converter folder

- **GadgetBridge**
    1. Bluetooth Payload Detail: https://www.espruino.com/Gadgetbridge
    2. Banglejs GadgetBrige Code
        2.1) https://codeberg.org/Freeyourgadget/Gadgetbridge/src/commit/c42cc91586a6e6d31863ee7fe637f01a5845941c/app/src/main/java/nodomain/freeyourgadget/gadgetbridge/service/devices/banglejs/BangleJSDeviceSupport.java

- **DEBUGGING**
    1. For enabling esp_pm_dump_locks, below config need to be set
        1. CONFIG_PM_PROFILING=y
        2. CONFIG_PM_TRACE=y
    Code Example: 
    ```
    #include "esp_pm.h"
    esp_pm_dump_locks(stdout);
    ```
- **Light Sleep Callbacks**
1. Firstly, set the below config
    1. CONFIG_PM_LIGHT_SLEEP_CALLBACKS=y

2. Code Example
```
static uint64_t sleep_enter_count = 0;
static uint64_t sleep_exit_count = 0;

static uint64_t total_sleep_us = 0;
static uint64_t last_sleep_start_us = 0;
static uint64_t longest_sleep_us = 0;

static int on_sleep_enter(int64_t sleep_time_us, void *arg)
{
    sleep_enter_count++;

    last_sleep_start_us = esp_timer_get_time();

    return 0;
}

static int on_sleep_exit(int64_t slept_time_us, void *arg)
{
    sleep_exit_count++;

    const uint64_t sleep_end_us = esp_timer_get_time();

    if (sleep_end_us >= last_sleep_start_us)
    {
        const uint64_t sleep_duration_us =
            sleep_end_us - last_sleep_start_us;

        total_sleep_us += sleep_duration_us;

        if (sleep_duration_us > longest_sleep_us)
        {
            longest_sleep_us = sleep_duration_us;
        }
    }
    return 0;
}

esp_pm_sleep_cbs_register_config_t cb_config = {
    .enter_cb = on_sleep_enter,
    .exit_cb = on_sleep_exit,
    .enter_cb_user_arg = NULL,
    .exit_cb_user_arg = NULL,
    .enter_cb_prior = 5, // Default mid-level priority
    .exit_cb_prior = 5   // Default mid-level priority
};

esp_err_t err = esp_pm_light_sleep_register_cbs(&cb_config);
if (err != ESP_OK)
{
    ESP_LOGE(TAG, "Failed to register sleep callbacks: %s", esp_err_to_name(err));
    return;
}
ESP_LOGI(TAG, "Sleep callbacks registered successfully.");
```
