#include "rpg_audio.h"

#include "bsp_audio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>

static const char *TAG = "rpg_audio";

#define SAMPLE_RATE   16000u
#define CHUNK_SAMPLES 256u

static TaskHandle_t s_task;
static volatile bool s_req;
static volatile uint32_t s_hz;
static volatile uint16_t s_ms;

static void play_tone(uint32_t hz, uint16_t ms)
{
    if (hz == 0 || ms == 0) return;

    if (bsp_audio_set_format(SAMPLE_RATE, 16, 1) != ESP_OK) {
        ESP_LOGW(TAG, "audio format failed");
        return;
    }
    bsp_audio_set_volume(70);

    int16_t *buf = malloc(CHUNK_SAMPLES * sizeof(int16_t));
    if (!buf) {
        ESP_LOGW(TAG, "audio buffer alloc failed");
        return;
    }

    uint32_t total = (uint32_t)SAMPLE_RATE * ms / 1000u;
    uint32_t period = SAMPLE_RATE / hz;
    if (period == 0) {
        period = 1;
    }
    uint32_t phase = 0;

    while (total > 0) {
        uint32_t n = total < CHUNK_SAMPLES ? total : CHUNK_SAMPLES;
        for (uint32_t i = 0; i < n; i++) {
            buf[i] = (phase < period / 2u) ? 6000 : -6000;
            if (++phase >= period) {
                phase = 0;
            }
        }
        bsp_audio_write(buf, (size_t)n * sizeof(int16_t));
        total -= n;
    }

    free(buf);
}

static void audio_task(void *arg)
{
    (void)arg;
    for (;;) {
        if (s_req) {
            uint32_t hz = s_hz;
            uint16_t ms = s_ms;
            s_req = false;
            play_tone(hz, ms);
        } else {
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}

esp_err_t rpg_audio_start(void)
{
    if (s_task) {
        return ESP_OK;
    }

    esp_err_t err = bsp_audio_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "audio init failed: %s", esp_err_to_name(err));
        return err;
    }

    if (xTaskCreate(audio_task, "rpg_audio", 4096, NULL, 4, &s_task) != pdPASS) {
        ESP_LOGW(TAG, "audio task create failed");
        return ESP_FAIL;
    }
    return ESP_OK;
}

void rpg_audio_stop(void)
{
    s_req = false;
    if (s_task) {
        vTaskDelete(s_task);
        s_task = NULL;
    }
}

void rpg_audio_play_tone(uint32_t hz, uint16_t ms)
{
    s_hz = hz;
    s_ms = ms;
    s_req = true;
}
