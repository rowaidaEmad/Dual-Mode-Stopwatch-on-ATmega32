# ⏱️ Dual-Mode Stopwatch on ATmega32 (Count-Up + Count-Down)

A digital stopwatch built on **ATmega32** that supports **two modes**:

- **Count-Up (default)** — increments from `00:00:00`
- **Count-Down** — user sets a start time; a **buzzer** alerts at zero

Time is shown as **HH:MM:SS** on **six multiplexed 7-segment digits** driven through a **BCD-to-7-segment decoder (7447)**.  
Timing uses **Timer1 in CTC mode @ 16 MHz**, and **external interrupts** handle **Reset / Pause / Resume**. Mode LEDs indicate **up / down**.

> Firmware is written in **Embedded C** for AVR.

---

## 📸 Demo
https://github.com/rowaidaEmad/Dual-Mode-Stopwatch-on-ATmega32/blob/main/Stopwatch%20dual%20Demo.mp4 




---

## ✨ Features

- Two modes: **Count-Up** and **Count-Down** (toggle by a button)
- **Reset / Pause / Resume** via external interrupts (fast and reliable)
- **H/M/S adjustment** for Count-Down while paused (no accidental edits while running)
- **Buzzer** when Count-Down reaches 0
- **Mode LEDs**: Up-mode LED and Down-mode LED
- **Six-digit 7-segment display** (HH:MM:SS) via **multiplexing** to save pins
- Clean handling of edge cases: de-bounced inputs, no negative time, safe state transitions

---

## 🔌 Hardware Overview

- **MCU:** ATmega32 @ **16 MHz**
- **Display:** 6 × 7-segment (common-anode), shared segments via **7447** BCD decoder  
  (7447 outputs are active-low; segments are sink-driven)
- **Digit enable:** 6 MCU pins switch the common anodes (via transistor drivers) for **multiplexing**
- **Buzzer** output
- **Mode LEDs**
- **Buttons:** Reset, Pause, Resume, Mode Toggle, and **±** for Hours/Minutes/Seconds

### Suggested Pin Map

> Adjust to match your wiring if it differs; keep the table updated.

| Function                     | MCU Port/Pin | Notes |
| ---                          | ---          | --- |
| BCD to 7447 (D0..D3)         | **PC0..PC3** | Sends BCD for current digit |
| Digit Enable (6 digits)      | **PA0..PA5** | One pin per digit (via transistor drivers) |
| **Buzzer**                   | **PD0**      | Active when Count-Down reaches zero |
| **Up-mode LED**              | **PD4**      | Lit in Count-Up mode |
| **Down-mode LED**            | **PD5**      | Lit in Count-Down mode |
| **Reset**                    | **PD2 / INT0** | External interrupt |
| **Pause**                    | **PD3 / INT1** | External interrupt |
| **Resume**                   | **PB2 / INT2** | External interrupt |
| **Mode Toggle (Up/Down)**    | **PB7**      | Normal GPIO |
| **Hours + / –**              | **PB1 / PB0** | Normal GPIO (internal pull-ups) |
| **Minutes + / –**            | **PB4 / PB3** | Normal GPIO (internal pull-ups) |
| **Seconds + / –**            | **PB6 / PB5** | Normal GPIO (internal pull-ups) |

> **Debouncing:** hardware RC or simple software debounce (e.g., 10–20 ms) is recommended.

---

## 🧠 Firmware Architecture

### State Machine

- **IDLE / RUN_UP / RUN_DOWN / PAUSED**
- **Events:** `RESET`, `PAUSE`, `RESUME`, `MODE_TOGGLE`, `ADJUST(H/M/S, ±)`, `TICK_1S`, `REACHED_ZERO`
- Mode LEDs reflect the active mode; adjustments are only applied when **paused**.

### Timing

- **Timer1 (CTC)** generates a 1 Hz “seconds” tick.
- **Multiplex Refresh** runs fast (e.g., ~1 kHz overall → ~166 Hz per digit).  
  Implement via a lightweight timer ISR or a tight loop; each tick:
  1) Enable next digit,
  2) Output its BCD to PC0..PC3,
  3) Brief on-time (hundreds of microseconds),
  4) Disable and move to the next digit.

#### 1 Hz math (common setup)

- System clock: **16 MHz**
- Prescaler: **1024**
- Timer1 tick: `16,000,000 / 1024 = 15,625 Hz`
- For 1 Hz in **CTC**: set **OCR1A = 15624** (counts 0..15624 → 15,625 ticks)

### Time Representation

Keep time in three bytes: `hours (0..99)`, `minutes (0..59)`, `seconds (0..59)`  
- **Normalize** after each change (carry/borrow across fields)
- **Count-Down** stops at `00:00:00`, triggers buzzer, and transitions to **PAUSED** (or **IDLE**) to prevent negative values

---

## 🔍 Function Reference (typical layout)

> Names may differ in your code; update to match your actual functions.

- **`void gpio_init(void)`** — set DDRx, enable pull-ups for buttons, configure LEDs/buzzer.
- **`void timer1_init_ctc_1hz(void)`** — CTC mode, prescaler 1024, `OCR1A=15624`, enable compare-match ISR.
- **`void display_init(void)`** — configure digit-enable pins and 7447 BCD lines.
- **`void display_scan_tick(void)`** — one step of multiplexing (advance digit and output BCD).
- **`void display_write_hh_mm_ss(void)`** — map current time to 6 digits and call `display_scan_tick()` repeatedly.
- **`void time_increment_1s(void)`** — `ss++` with carry to `mm`, then `hh`.
- **`void time_decrement_1s(void)`** — `ss--` with borrow; clamp at zero.
- **`void time_adjust(enum unit {H,M,S}, int delta)`** — apply ±1 (while paused); normalize.
- **`ISR(TIMER1_COMPA_vect)`** — fires every second: advance or decrement time based on mode (when running).
- **`ISR(INT0_vect)`** — **Reset**: zero time; stop buzzer; go to **PAUSED**.
- **`ISR(INT1_vect)`** — **Pause**: capture state → **PAUSED**.
- **`ISR(INT2_vect)`** — **Resume**: return to **RUN_UP** or **RUN_DOWN**.
- **`void set_mode(enum {UP, DOWN})`** — switch LEDs, guard adjustments.

---
## 🧪 Simulation (Proteus)

This project was simulated and verified in **Proteus**.

- **New Project:** 
- **MCU:** ATmega32 @ **16 MHz**
- **Display:** 6× 7-segment via **7447** BCD decoder, multiplexed by digit-enable transistors
- **Inputs:** Reset (INT0), Pause (INT1), Resume (INT2), Mode toggle, H/M/S ±
- **Outputs:** Mode LEDs, buzzer on zero

### Run in Proteus
1. Open `proteus/Stopwatch.pdsprj`.
2. Confirm MCU clock = **16 MHz**.
3. Load the generated HEX (`build/stopwatch.hex`) to the ATmega32 device.
4. Press **Run**; use the buttons to pause/resume, toggle mode, and adjust H/M/S (when paused).

> All **Proteus files** are included so you can reproduce the exact demo wiring and behavior.

---
## 🧪 Usage

1. **Power on** → **Count-Up** starts automatically (Up LED on).  
2. **Pause** to stop counting; LEDs retain the current mode.  
3. **Mode Toggle** to switch to **Count-Down**.  
4. While **paused** in Count-Down, adjust **H/M/S** with the ± buttons.  
5. **Resume** to start counting down.  
6. On **zero**, buzzer sounds and the stopwatch stops (prevent negative time).  
7. **Reset** clears time to `00:00:00`.



