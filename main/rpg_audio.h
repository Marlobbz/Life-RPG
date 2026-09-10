// main/rpg_audio.h —— Life RPG 简短音效播放器。
#pragma once

#include "esp_err.h"
#include <stdint.h>

// 初始化音频并启动后台播放任务。失败时可忽略,Life RPG 仍可无声音运行。
esp_err_t rpg_audio_start(void);

// 停止并删除后台播放任务。
void rpg_audio_stop(void);

// 请求播放一段短方波音效。实际 PCM 写入在工作任务中执行。
void rpg_audio_play_tone(uint32_t hz, uint16_t ms);
