// main/rpg_date.h —— Life RPG 日期抽象。
//
// 当前设备没有可靠的墙上时间源,业务层不直接依赖 RTC。
// 上层通过 rpg_date_set_provider() 注入日期来源;开发模式可注入模拟日期。
#pragma once

#include <stdint.h>

typedef struct {
    uint16_t year;
    uint8_t month; // 1..12
    uint8_t day;   // 1..31
} rpg_date_t;

typedef void (*rpg_date_provider_t)(rpg_date_t *out);

// 设置当前日期来源。没有设置时使用默认开发日期。
void rpg_date_set_provider(rpg_date_provider_t provider);

// 把当前日期写入 out。
void rpg_date_today(rpg_date_t *out);

// 把日期转成从 1970-01-01 开始的天序号,用于连续天数比较。
uint32_t rpg_date_to_serial(const rpg_date_t *date);
