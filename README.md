# ⏱️ Dual-Mode Stopwatch on ATmega32 (Increment + Countdown)

A digital stopwatch for **ATmega32** with **two modes**:
- **Increment (default)** — counts up from 00:00:00
- **Countdown** — user sets a start time; buzzer fires at zero

Six **multiplexed** 7-segment displays (HH:MM:SS) driven via a **7447 BCD decoder**.  
Inputs provide **Reset / Pause / Resume / Mode Toggle** and **H:M:S adjustments** in countdown mode.  
Mode LEDs show count-up vs. count-down status; a buzzer alerts at the end. :contentReference[oaicite:2]{index=2}

---

## ✨ Features
- **Two modes:** increment & countdown (toggle by button) :contentReference[oaicite:3]{index=3}
- **Reset / Pause / Resume** via external interrupts (INT0/INT1/INT2) :contentReference[oaicite:4]{index=4}
- **Adjust H/M/S** for countdown before starting (while paused) :contentReference[oaicite:5]{index=5}
- **Buzzer** when countdown reaches zero; **LEDs** show active mode :contentReference[oaicite:6]{index=6}
- **Robust timing** using **Timer1 in CTC mode** at **16 MHz** system clock :contentReference[oaicite:7]{index=7}
- **Efficient display** using 6-digit multiplexing with NPN transistors to enable each digit :contentReference[oaicite:8]{index=8}

---

## 🧩 Hardware Overview

- **MCU:** ATmega32 @ **16 MHz**  
- **Display:** 6× 7-segment (common anode) via **7447 BCD decoder** (PORTC[3:0]) :contentReference[oaicite:9]{index=9}
- **Digit enable (multiplex):** 6 pins on **PORTA** (each drives an NPN to switch a digit) :contentReference[oaicite:10]{index=10}
- **Buzzer/Alarm:** **PD0** (active at countdown complete) :contentReference[oaicite:11]{index=11}
- **Mode LEDs:** **PD4 = counting up (red)**, **PD5 = counting down (yellow)** :contentReference[oaicite:12]{index=12}

**Button mapping** (internal/external resistors per design): :contentReference[oaicite:13]{index=13}
- **Reset:** **PD2 / INT0** (internal pull-up, falling edge)
- **Pause:** **PD3 / INT1** (external pull-down, rising edge)
- **Resume:** **PB2 / INT2** (internal pull-up, falling edge)
- **Mode Toggle:** **PB7**
- **Adjust Hours:** **PB1 = +**, **PB0 = –** (internal pull-up)
- **Adjust Minutes:** **PB4 = +**, **PB3 = –** (internal pull-up)
- **Adjust Seconds:** **PB6 = +**, **PB5 = –** (internal pull-up)

---

## 🕹️ Operation

- **Increment mode (default):** starts counting up at power-on; **PD4 LED** lit. :contentReference[oaicite:14]{index=14}  
- **Toggle to countdown:** press Mode button (**PB7**), set **H/M/S** while **paused**, then **Resume** to start; **PD5 LED** lit. Buzzer on zero. :contentReference[oaicite:15]{index=15}

---

## 🛠️ Firmware Notes

- **Timer1 (CTC)** generates precise ticks for seconds counting.  
- **External interrupts** (INT0/1/2) handle real-time user actions without polling lag.  
- **Multiplexing**: only one digit is lit at a time; rapid scanning + POV makes all appear on. Use **NPN BJTs** to switch common anodes; 7447 provides BCD→7-seg decoding on the shared segment lines. :contentReference[oaicite:16]{index=16}



# flash (example programmer/port):
avrdude -p m32 -c usbasp -U flash:w:stopwatch.hex
