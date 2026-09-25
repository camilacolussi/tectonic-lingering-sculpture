# Hardware design

The physical build: enclosures, power distribution, cabling and connectors.
Electronics and code decisions live in each system's notes file; this
document covers the parts you build, cut, wire and mount.

## Power supply enclosure (System 1)

A laser-cut MDF box holding the 5V 30A supply that powers the 3 LED strips
(40 LEDs each). Mains comes in through a switch; the supply's 5V output is
split to the 3 strip power cables inside the box.

Photo: `media/Making_connections1.jpg`.

### How much current each strip draws

Worked out before choosing connectors, since it sets what they must handle:

- WS2812B LED at full white: ~60 mA.
- 40 LEDs x 60 mA = **2.4 A per strip** worst case (full white, brightness 255).
- **~1.4 A per strip** at the brightness cap used in the code (150/255).
- Full-white flash lasts **3 min at most**; the wave pattern the rest of the
  time draws much less.

So each strip connection needs to be comfortable at ~3 A. Current turned
out not to be the hard constraint. Cable size was.

### Strip power cables

- Conductors: 14–18 AWG (to confirm; printed on the inner insulation).
- Outside diameter: **~10 mm** (double insulation + cloth braid).
- Power only (5V + GND). Data runs on separate wires.

### Connecting the strip cables: Wago lever connectors (decided 2026-09-25)

**Tried first: 5.5 x 2.1 mm panel-mount DC barrel jacks**
(`media/Making_connections2.jpg`). Rejected because:
- the 10 mm cable doesn't fit the male plug;
- cheap barrel jacks are rated only ~2–5 A, and their small spring contacts
  heat up near that limit. That is borderline for 2.4 A.

**Chosen: Wago 221 lever connectors inside the box.**
- Rated ~20–32 A, far above 2.4 A.
- Take 0.2–4 mm² wire (~24–12 AWG), so 14–18 AWG fits.
- Can still be disconnected without tools (lift the lever), just not from
  outside the box. Accepted trade-off: not plug-and-play, but simple and
  reliable.

**Alternatives considered**, if plugging from outside the box is wanted later:

| Option | Why it would work | Trade-off |
|---|---|---|
| XT60 inline | ~£1, polarised, 30 A, takes thick wire | Solder; sits on the cable, not the box |
| Neutrik speakON NL2 | Installation/stage standard for low-voltage DC; locking; takes 6–15 mm cable whole; screw terminals | ~£4–8 each; needs a 24 mm panel hole; label "5V DC" so it isn't confused with a speaker socket |
| Short thin pigtail + 5.5 x 2.5 mm barrel plug rated 5 A | Keeps the barrel-jack box design | Splice joint; plugs pull out easily |
| IP68 2-pin screw-terminal inline connector | ~£2, glands take 5–10 mm cable | Inline, fiddlier to reconnect |

### Strain relief: M20 nylon cable glands (ordered 2026-09-25, not yet tested)

**Why it's needed:** Wagos grip wire well but aren't built to take a pull.
Without strain relief, a tug on a cable (someone tripping on it) goes
straight to the connector. Strain relief clamps the cable's outer jacket at
the box wall so the force stops there.

**Options considered:**
- Cable gland: screws through the wall; its cap squeezes a seal around the
  cable. Neatest and strongest. **Chosen.**
- Zip tie + screw-in mount inside the wall: cheapest, no change to the box.
- P-clip / cable clamp screwed inside the wall: firm and removable.

**How the gland size was picked:**
1. Clamping range must put the cable's outer diameter (10 mm) in the
   *middle*, not at the edge. At the top of the range it won't seal; at the
   bottom it won't grip. **M20 (~6–12 mm)** fits; PG11 (~5–10 mm) is too
   tight.
2. Hole size is the thread size: **20 mm hole** for M20.
3. Thread must be longer than the wall is thick so the locknut can bite.
   3–6 mm MDF is fine.
4. Nylon is enough indoors; metal only for outdoor or rough use.

**Test once fitted:** pull each cable firmly from outside. Nothing inside
the box should move.

### Safety

- A 30 A supply can push a lot of current into a short circuit. A fuse on
  the 5V line and wire rated for the current are worth adding.

## Open items

- [x] Decide how strip power cables connect at the box: Wago lever connectors inside
- [ ] Confirm strip cable conductor size (14 or 18 AWG)
- [ ] Cut 20 mm holes and fit the M20 cable glands; pull-test each cable
- [ ] Decide on and fit a fuse on the 5V line
- [ ] Add enclosure cutting files and final photos to the repo
