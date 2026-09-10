#include "fap_screenshot.h"

#include "bsp_display.h"
#include "driver/usb_serial_jtag.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "src/draw/snapshot/lv_snapshot.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "fap_screenshot";
static TaskHandle_t s_task;

static void write_all(const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    size_t off = 0;
    while (off < len) {
        int n = usb_serial_jtag_write_bytes(p + off, len - off, pdMS_TO_TICKS(5000));
        if (n <= 0) {
            ESP_LOGW(TAG, "screenshot write failed");
            return;
        }
        off += (size_t)n;
    }
}

static void capture_and_send(void)
{
    lv_draw_buf_t *draw_buf = NULL;

    if (!bsp_lvgl_lock(1000)) {
        ESP_LOGW(TAG, "LVGL lock failed");
        return;
    }

    lv_obj_t *screen = lv_screen_active();
    if (screen) {
        draw_buf = lv_snapshot_take(screen, LV_COLOR_FORMAT_RGB565);
    }
    bsp_lvgl_unlock();

    if (!draw_buf) {
        ESP_LOGW(TAG, "snapshot failed");
        return;
    }

    char header[96];
    int header_len = snprintf(header, sizeof(header),
                              "FAP_SCREENSHOT_V1 %u %u RGB565LE %u\n",
                              (unsigned)draw_buf->header.w,
                              (unsigned)draw_buf->header.h,
                              (unsigned)draw_buf->data_size);
    if (header_len > 0 && (size_t)header_len < sizeof(header)) {
        write_all(header, (size_t)header_len);
        write_all(draw_buf->data, draw_buf->data_size);
    }

    lv_draw_buf_destroy(draw_buf);
}

static void screenshot_task(void *arg)
{
    (void)arg;
    char line[64];
    size_t used = 0;

    for (;;) {
        uint8_t byte;
        int n = usb_serial_jtag_read_bytes(&byte, 1, pdMS_TO_TICKS(50));
        if (n == 1) {
            if (byte == '\n') {
                if (used > 0 && line[used - 1] == '\r') {
                    line[--used] = '\0';
                } else {
                    line[used] = '\0';
                }
                if (strcmp(line, "FAP_SCREENSHOT_V1") == 0) {
                    capture_and_send();
                }
                used = 0;
            } else if (used + 1 < sizeof(line)) {
                line[used++] = (char)byte;
            } else {
                used = 0;
            }
        } else if (n < 0) {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

esp_err_t fap_screenshot_start(void)
{
    if (s_task) {
        return ESP_OK;
    }
    if (xTaskCreate(screenshot_task, "fap_shot", 4096, NULL, 5, &s_task) != pdPASS) {
        return ESP_FAIL;
    }
    return ESP_OK;
}
