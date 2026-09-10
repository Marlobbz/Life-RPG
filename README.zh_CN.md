<p align="right">
  <strong>简体中文</strong> · <a href="README.md">English</a>
</p>

# Life RPG

Life RPG 把 FoloToy AI Passport 变成一台口袋大小的生活 RPG。背单词、运动、刷题、读书、走路这些现实里的事就是任务：完成任务可以拿经验、提升角色属性、延续连续天数，并养活一只小小的像素宠物。

设备不需要手机、不需要账号、不需要联网，也不需要 AI 服务：进度存在板子本地，断电也不会丢。

本仓库是 FoloToy AI Passport 官方基线的一个 fork。固件开机直接进入 Life RPG，不再走官方 demo 菜单。上游项目介绍见 [上游基线](#上游基线folotoy-ai-passport-固件)。

## 好玩在哪里

- **现实就是游戏世界。** 任务都是你今天真要做的事：背 30 个单词、运动 2 小时、解 1 道 CTF。
- **一件事两份收益。** 完成任务既给经验，也涨一项角色属性（BODY / CODE / KNOWLEDGE），角色面板会慢慢长成你真实习惯的样子。
- **宠物替你把账记着。** 每摸鱼一天，心情和精力都会掉一截；每完成一个任务，又会回来一点。偷懒是会被屏幕看见的。
- **连续天数让中断很贵。** 当天完成任意一个任务就算续上；整天没动，连续天数直接清零。
- **反馈来得又快又直接。** 三个按键，不用在菜单里翻来翻去：经验条带动画、完成任务放烟花、升级时“LEVEL UP!” 从屏幕中间飞过。

## 游戏系统

| 系统 | 行为 | 数值 |
| --- | --- | --- |
| Player | 等级、当前等级经验、生涯总经验、三项属性 | 1 级升 2 级需 100 XP，之后每级需求 ×1.5；属性独立累计且不随升级清零 |
| 每日任务 | 固定 7 个现实任务的清单，每个只结算一次 | 单个任务 10–40 XP、+1 至 +3 属性点，全部完成共 155 XP |
| Streak | 至少完成一个任务的连续天数 | 同一天只算一次；断档两天及以上清零 |
| Pet | 心情、精力、信任随行为变化，离线期间衰减 | 初始 70/80/0；完成任务 +10 心情、+5 精力、+5 信任；每漏一天 −5 心情、−10 精力，最多累计 10 天 |
| 宠物状态 | 一眼看出宠物现在过得怎么样 | 心情 ≥70 且精力 ≥50 为 HAPPY；精力 <30 或心情 <20 为 TIRED；其余为 NORMAL |

### 任务清单

| 任务 | 奖励 | 属性 |
| --- | --- | --- |
| Memorize 30 words | +20 XP | KNOWLEDGE +2 |
| Exercise 2 hours | +40 XP | BODY +3 |
| Code 1 problem | +25 XP | CODE +2 |
| Study 30 min | +20 XP | KNOWLEDGE +1 |
| Read 10 pages | +10 XP | KNOWLEDGE +1 |
| Walk 3000 steps | +10 XP | BODY +1 |
| Solve 1 CTF | +30 XP | CODE +2 |

清单固定并持久化，不按日期轮换：它更像一张你自己选定的作息清单，而不是抽奖。

## 操作方式

全部输入只有三个物理按键：`UP`、`DOWN`、`OK`。任意按键双击退回上一层。

| 页面 | UP / DOWN | OK | 双击 |
| --- | --- | --- | --- |
| `HOME` | 在 `PLAYER`、`QUEST`、`PET` 之间选择 | 进入选中页面 | — |
| `PLAYER` | 调节开发用日期 | — | 返回 `HOME` |
| `QUESTS` | 在任务列表中移动 | 打开任务详情 | 返回 `HOME` |
| `QUEST` | — | 完成任务 | 返回任务列表 |
| `PET` | — | — | 返回 `HOME` |

## 各页面

- `HOME` —— 像素风标题卡，三个入口、电量显示，以及每次按键都会跳一下的吉祥物。
- `PLAYER` —— `LV.nn`、当前/所需经验、带动画的经验条、当前连续天数（并标出今天 `DONE` 还是 `OPEN`），以及 BODY / CODE / KNOWLEDGE。
- `QUESTS` —— 7 个任务的滚动窗口，已完成的前面标 `[X]`。
- `QUEST` —— 任务标题、描述、XP 奖励和完成状态；按 `OK` 即完成任务。
- `PET` —— 心情、精力、信任，以及推导出的状态。

## 数据与供电

- Player、任务、Streak、Pet 状态都存在固件自己的 NVS 命名空间里，完成任务时和离开应用页时写入。断电、抠电池都不会丢进度。
- 重置 Life RPG 数据只会清除它自己的键，绝不会碰受保护的设备身份分区和其他应用数据。
- 板子没有可靠的墙上时钟，日期来自可注入的 provider（`rpg_date_set_provider`）。当前固件使用开发用日期来源，可在 `PLAYER` 页面用 `UP`/`DOWN` 手动调节。以后接入 RTC 或 SNTP 时只替换 provider，不改游戏逻辑。

## 本版本不包含

- 没有音效：为了在当前硬件上保持固件体积小、运行稳定，音效被移除。
- 运行时没有云同步、账号、联网或 AI 功能。Life RPG 完全离线，AI 能力只作为可选增强，而不是依赖。
- 官方 demo 页面仍然链进固件以便查阅，但从主流程中进不去。

## 构建与刷机

使用 ESP-IDF 5.5.3，目标芯片 ESP32-C3：

```bash
source <path-to-esp-idf-5.5.3>/export.sh
idf.py --version                  # 必须显示 ESP-IDF v5.5.3
./tools/validate.sh --firmware    # 构建并校验合并镜像
```

用官方浏览器刷机工具把校验通过的 `build/FoloToy-AI-Passport-full.bin` 从 `0x0` 写入（<https://ai-passport.folotoy.cn/tools/web-flasher/>，波特率 `460800`）。合并镜像覆盖整个 Flash 布局，因此只适用于空白设备；已经烧录过官方固件的设备请烧应用分区，不要把整镜像写进受保护的设备身份区。

## 项目结构

```text
main/rpg_player.{c,h}     Player、经验曲线、升级、属性逻辑（不依赖 ESP-IDF 与 LVGL）
main/rpg_quest.{c,h}      任务清单与完成判定
main/rpg_streak.{c,h}     连续天数记账
main/rpg_pet.{c,h}        宠物状态、正向反馈与每日衰减
main/rpg_date.{c,h}       日期抽象与可注入 provider
main/rpg_storage.{c,h}    游戏状态的 NVS 持久化
main/demo_life_rpg.c      Life RPG 的 LVGL 页面与按键分发
main/ui_pixel.{c,h}       与官方 demo 共用的像素风 UI 组件
tests/test_rpg_*.c        不需要硬件的纯逻辑主机测试
docs/                     上游基线文档，保持原样
```

游戏逻辑刻意留在不依赖 ESP-IDF 与 LVGL 的纯 C 模块里，既能直接在主机上编译测试，也保证 Life RPG 的代码模块化、可整体移除。

## 测试

```bash
./tools/validate.sh --static   # 仓库检查 + 主机逻辑测试
```

主机测试覆盖经验与升级曲线、任务奖励与重复完成保护、连续天数状态转换、宠物数值饱和与衰减，以及像素布局计算。

## 上游基线：FoloToy AI Passport 固件

Life RPG 构建在官方 FoloToy AI Passport 仓库之上，硬件、BSP 接口与验证流程以该仓库为准。以下摘自官方 README [docs/README.md](docs/README.md)：

> FoloToy AI Passport is open wearable AI hardware. This repository is the
> development baseline for the device. It keeps the **hardware facts, stable
> interfaces, resource boundaries, reference implementations, and validation
> methods** needed to build applications in one place.
>
> The repository is organized around the following principles:
>
> - `main` is the smallest complete runnable baseline and an executable
>   description of the current hardware capabilities.
> - `components/bsp` isolates board-level details and exposes stable APIs to
>   applications.
> - `demo/*` branches show different paths from a product requirement to a
>   working implementation.
> - Development conventions for AI assistants live in `AGENTS.md` and
>   `docs/development/ai-guide.md`; the complete hardware context and
>   troubleshooting knowledge is in
>   `docs/hardware-design/AI_HARDWARE_DEVELOPMENT_GUIDE.md`.
> - Build results and physical-device results are reported separately. A
>   successful build must never be presented as successful hardware validation.

完整官方文档：[英文](docs/README.md) · [简体中文](docs/README.zh_CN.md)。fork 工作流与 fork 内容归属见 [docs/fork-guide.zh_CN.md](docs/fork-guide.zh_CN.md)。

## 许可

MIT，© 2026 FoloToy，见 [LICENSE](LICENSE)。仓库开发规则见 [AGENTS.zh_CN.md](AGENTS.zh_CN.md)。
