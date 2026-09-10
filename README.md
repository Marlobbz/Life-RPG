<p align="right">
  <a href="README.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# Life RPG

Life RPG turns the FoloToy AI Passport into a pocket-sized life RPG. Real-world
actions — memorizing words, working out, solving a coding problem, reading,
walking — are quests that grant XP, raise character stats, extend a daily
streak, and keep a small pixel pet alive.

The device needs no phone, no account, no network, and no AI service: progress
is stored on the board itself and survives power loss.

This repository is a fork of the official FoloToy AI Passport baseline. The
firmware boots straight into Life RPG and no longer uses the demo menu flow. The
upstream project overview is quoted in
[Upstream baseline](#upstream-baseline-the-folotoy-ai-passport-firmware).

## What makes it playable

- **The real world is the game world.** Quests are things you actually do
  today: memorize 30 words, work out for two hours, solve one CTF challenge.
- **Every action pays out twice.** A finished quest grants XP and pushes one
  character stat — BODY, CODE, or KNOWLEDGE — so the character sheet slowly
  starts to look like your own habits.
- **The pet keeps score for you.** Mood and energy drop for every day you skip
  and recover a little for every quest you finish. Ignoring the device becomes
  visible on its screen.
- **Streaks make skipping expensive.** One completed quest keeps the day alive;
  miss a whole day and the counter resets to zero.
- **Feedback is immediate.** Three buttons, no menu digging: the XP bar fills
  with an animation, fireworks pop when a quest is completed, and "LEVEL UP!"
  flies across the screen on the one level that matters.

## Game systems

| System | Behaviour | Numbers |
| --- | --- | --- |
| Player | Level, XP toward the next level, lifetime XP, three stats | Level 1 → 2 needs 100 XP, each later level asks 1.5× more; stats accumulate separately and never reset |
| Daily quests | A fixed catalog of seven real-life tasks, each awarded once | 10–40 XP and +1 to +3 stat points per quest, 155 XP for the full board |
| Streak | Days in a row with at least one completed quest | Same-day completions count once; a gap of two days or more clears the streak |
| Pet | Mood, energy, and trust react to your behaviour and decay while you are away | Starts at 70/80/0; a quest gives +10 mood, +5 energy, +5 trust; each missed day costs 5 mood and 10 energy, capped at 10 days |
| Pet state | One glance tells you how the pet is doing | HAPPY at 70+ mood and 50+ energy; TIRED below 30 energy or 20 mood; NORMAL in between |

### The quest catalog

| Quest | Reward | Stat |
| --- | --- | --- |
| Memorize 30 words | +20 XP | KNOWLEDGE +2 |
| Exercise 2 hours | +40 XP | BODY +3 |
| Code 1 problem | +25 XP | CODE +2 |
| Study 30 min | +20 XP | KNOWLEDGE +1 |
| Read 10 pages | +10 XP | KNOWLEDGE +1 |
| Walk 3000 steps | +10 XP | BODY +1 |
| Solve 1 CTF | +30 XP | CODE +2 |

The catalog is fixed and persisted rather than shuffled every day, so the board
behaves like a checklist for a routine you chose, not a lottery.

## Controls

All input is three physical buttons: `UP`, `DOWN`, and `OK`. A double press of
any button steps back one screen.

| Screen | UP / DOWN | OK | Double press |
| --- | --- | --- | --- |
| `HOME` | Select `PLAYER`, `QUEST`, or `PET` | Enter the selected screen | — |
| `PLAYER` | Move the development date | — | Back to `HOME` |
| `QUESTS` | Move through the catalog | Open the quest detail | Back to `HOME` |
| `QUEST` | — | Complete the quest | Back to the quest list |
| `PET` | — | — | Back to `HOME` |

## Screens

- `HOME` — pixel-art title card with the three destinations, the battery level,
  and a mascot that jumps on every key press.
- `PLAYER` — `LV.nn`, current and required XP, an animated XP bar, the current
  streak with a `DONE`/`OPEN` marker for today, and BODY / CODE / KNOWLEDGE.
- `QUESTS` — a scrolling window of the seven quests with `[X]` markers for the
  ones already finished.
- `QUEST` — title, description, XP reward, and completion status; pressing `OK`
  completes the quest.
- `PET` — mood, energy, trust, and the derived state.

## Data and power

- Player, quest, streak, and pet state are stored in NVS in the firmware's own
  namespace and are written when a quest is completed and when the app page is
  left. Progress survives power loss and battery removal.
- Resetting Life RPG data clears only its own keys. The protected identity
  partition and the other applications' data are never touched.
- The board has no reliable wall clock, so dates come from an injectable
  provider (`rpg_date_set_provider`). The current build ships a development
  provider, adjustable with `UP`/`DOWN` on the `PLAYER` screen. Wiring an
  RTC or SNTP later replaces the provider without touching game logic.

## Not in this release

- No audio cues: sound effects were removed to keep the firmware small and
  stable on the current hardware.
- No cloud sync, account, network, or AI feature at runtime. Life RPG is fully
  offline, and AI features stay an optional enhancement rather than a
  dependency.
- The official demo pages remain linked into the image for reference, but they
  are not reachable from the main flow.

## Build and flash

Use ESP-IDF 5.5.3 for the ESP32-C3 target:

```bash
source <path-to-esp-idf-5.5.3>/export.sh
idf.py --version                  # must report ESP-IDF v5.5.3
./tools/validate.sh --firmware    # build plus merged-image verification
```

Flash the verified `build/FoloToy-AI-Passport-full.bin` from address `0x0` with
the official browser flasher
(<https://ai-passport.folotoy.cn/tools/web-flasher/>, baud `460800`). The merged
image covers the whole Flash layout, so use it only on a blank target; on an
already provisioned device, flash the application partitions instead of writing
the whole image over the protected identity area.

## Project layout

```text
main/rpg_player.{c,h}     Player, XP curve, level-up, stats (no ESP-IDF or LVGL)
main/rpg_quest.{c,h}      Quest catalog and completion rules
main/rpg_streak.{c,h}     Daily streak bookkeeping
main/rpg_pet.{c,h}        Pet state, feedback, and daily decay
main/rpg_date.{c,h}       Date abstraction with an injectable provider
main/rpg_storage.{c,h}    NVS persistence for the game state
main/demo_life_rpg.c      LVGL screens and key dispatch for Life RPG
main/ui_pixel.{c,h}       Pixel-art UI kit shared with the baseline demos
tests/test_rpg_*.c        Host tests for the game logic, no hardware required
docs/                     Upstream baseline documentation, unchanged
```

The game logic is deliberately kept in plain C modules that compile and test
without ESP-IDF or LVGL, which keeps the Life RPG code modular and removable.

## Tests

```bash
./tools/validate.sh --static   # repository checks plus the host logic tests
```

The host tests cover the XP and level curve, quest rewards and double-completion
guards, streak transitions, pet saturation and decay, and the pixel layout math.

## Upstream baseline: the FoloToy AI Passport firmware

Life RPG is built on the official FoloToy AI Passport repository, which stays the
source of truth for the board, its BSP interfaces, and its validation workflow.
Quoted from the official README, [docs/README.md](docs/README.md):

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

Full official documentation: [English](docs/README.md) ·
[Simplified Chinese](docs/README.zh_CN.md). Fork workflow and where fork-owned
content belongs: [docs/fork-guide.md](docs/fork-guide.md).

## License

MIT, © 2026 FoloToy. See [LICENSE](LICENSE). See
[AGENTS.md](AGENTS.md) for the repository development rules.
