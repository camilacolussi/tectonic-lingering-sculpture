# Working with this project
 
## About me
I'm learning to work well with AI on creative physical computing
projects. Building a second, better version of an earlier installation called 'Tectonic Lingering',
now with more understanding of what I'm doing (previously had to ask for help).
 
## How I like to work — please follow this
- **Step by step, one stage at a time.** Don't jump ahead or solve later stages preemptively.
- **Explain the "why" before/alongside code**, not just working code with no explanation. I'm trying to build real understanding, not just get a working result.
- **Be honest about hardware/technical limitations up front**, even if it complicates the plan. Don't paper over a real difficulty to keep things simple — flag it, and explain the reasoning behind any workaround.
- **Ask before assuming** if a decision could go multiple ways — don't silently pick one.
- Log real decisions and reasoning in the corresponding system's notes file as we go.

## Project context
- **Architecture (changed 2026-08-06):** the project is built as **two independent electronic systems**, not one board running sequential stages. See [docs/roadmap.md](docs/roadmap.md) for the full reasoning, including the open question of whether/how touch and the LED pattern still interact now that they're on separate boards.
  - **System 1 — API + LEDs**: code in `system-1-api-leds/`, notes in [docs/system-1-api-leds-notes.md](docs/system-1-api-leds-notes.md). Target board: ESP32 V2 (not yet owned as of 2026-08-06 — being tested on the ESP32-S3 as a stand-in until it arrives).
  - **System 2 — Touch + Sound**: code in `system-2-sound-touch/`, notes in [docs/system-2-sound-touch-notes.md](docs/system-2-sound-touch-notes.md). Board: Adafruit ESP32-S3 Feather.
- Full plan, decisions made, and open questions: see [docs/roadmap.md](docs/roadmap.md)
- Step-by-step build tutorial (for others to follow, written for a stranger with no context): see [docs/tutorial.md](docs/tutorial.md) — grows alongside the work. Different from the per-system notes: the notes capture *why* decisions were made, while the tutorial explains *how* to do each step. Update both when completing a real step, don't just update one.
- Each system's notes file should end with an **Open items** checklist — check this before assuming what's done vs. still pending.
- Edit the documents in the docs folder if we take different decisions or what they say needs update.

## Current focus
System 1 (API + LEDs): LED wave pulse pattern confirmed working on
hardware. Next up is the API piece — check
[docs/system-1-api-leds-notes.md](docs/system-1-api-leds-notes.md)
(especially Open items) for exact current status before assuming where
things stand.
