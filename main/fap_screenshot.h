// main/fap_screenshot.h —— 官方社区发布所需的串口截图协议。
#pragma once

#include "esp_err.h"

// 启动截图监听任务。该任务只观察 USB Serial/JTAG,不修改设备状态。
esp_err_t fap_screenshot_start(void);
