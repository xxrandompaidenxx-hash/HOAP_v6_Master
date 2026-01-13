# HOAP_v6_Master
🚀 Release Note: v6.0-STABLE
Title: HOAP v6.0 - The Persistant Update Tag: v6.0

This release marks the transition from volatile memory to EEPROM-based persistence. HOAP v6.0 is now a standalone security solution that remembers its users even after power loss.

What's New:

Smart-Match Engine: Implemented 4-byte strict matching for fobs and 1-byte dynamic bypass for phones.

Memory Expansion: 50 total slots (40 Fob, 10 Phone).

Acoustic Motion: Soft-open/close now includes pitch-mapped buzzing.

Hardware Optimization: Fixed the 8th-column LCD ghosting glitch.

🔧 Troubleshooting & Calibration
If you notice the gate isn't hitting the table perfectly or the RFID is acting up, check these three things:

Servo Limits: If the gate doesn't close all the way, change GATE_CLOSED = 90 to 95 or 100.

Power Draw: If the LCD dims when the servo moves, you might need to power the servo with a separate 5V battery pack (just remember to connect the grounds).

The "8th Bar": If the LCD still shows a weird line on the right side, make sure your code never uses lcd.print() for a word longer than 7 letters.
