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
- Log real decisions and reasoning in the corresponding stage notes files as we go.
- As I am starting this project, only [docs/stage-1-notes.md](docs/stage-1-notes.md) has been created; as we move forward, create the next stage notes files in the docs folder. 

## Project context
- Full plan, decisions made, and open questions: see [docs/roadmap.md](docs/roadmap.md)
- Per-stage details and hardware notes: see [docs/stage-1-notes.md](docs/stage-1-notes.md) for the current stage, and add later per-stage note files as the project expands
- Step-by-step build tutorial (for others to follow, written for a stranger with no context): see [docs/tutorial.md](docs/tutorial.md) — grows alongside the stages. Different from the stage notes: the notes capture *why* decisions were made, while the tutorial explains *how* to do each step. Update both when completing a real step, don't just update one.
- Each stage notes file should end with an **Open items** checklist — check this before assuming what's done vs. still pending.
- Edit the documents in the docs folder if we take different decisions or what they say needs update. 
  
## Current stage
Stage 1 (light + touch), in progress, on the Adafruit ESP32-S3 Feather.
Check [docs/stage-1-notes.md](docs/stage-1-notes.md) (especially Open items) for exact current status before assuming where things stand. Immediate next step:
finish computer setup for ESP32-S3 development (Arduino IDE + board support, see [docs/tutorial.md](docs/tutorial.md)), then write a touch-sensing test sketch before adding any LED code.
