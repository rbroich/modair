Different Configurations:

Option 1: 5.8" WITSCHI_ERM25664AAG-1 EastRising 256x64 pixel graphic LCD module Blue backlight.
-LCD goes on top of PCB
-Headers row on the left aligns
-Mounting holes on the left align
-Top right mounting hole aligns with frame ground pad on LCD (solder spacer?)
-Electrolytic Caps C4 and C5 can be bent over
-Contrast Potentiometer not needed (already on LCD module)
-Rot Encoder header attached to Bottom
-LED (or header?) soldered on Bottom
-Screw Terminals attached to Bottom



Option 2: 2.9" NelyTech NT-G128641A 128x64 Graphic LCD Display Module 2xKS0108 Black on White.
-LCD goes on top of PCB
-Headers row on the bottom aligns
-All mounting holes align
-Bend electrolytic caps C4 and C5 over
-Contrast Potentiometer soldered
-Rot Encoder header attached to Bottom
-LED (or header?) soldered on Bottom
-Screw Terminals attached to Bottom



Option 3: 3.1" Generic 5V 12864B V2.0 LCD Display Module 128x64 Dots Graphic Matrix LCD Blue Backlight
-Identical mounting to Option 2
-CS1 becomes PSB [parallel mode] (ST7920, 2MHz clock rate)
-CS2 is not used
-Contrast Potentiometer not needed (already on LCD module)
-Backlight Resistor already on LCD module. Drive with 5V



Option 4: Generic HD44780 2x16 Character LCD Blue Backlight
-Cut PCB Smaller
-LCD goes on top of PCB, rotated 180 degrees
-Headers row on the bottom right aligns
-All mounting holes align
-Rot Encoder mounts vertically on top left PCB (isolate exposed header pins underneath...gnd plate on bottom of rotenc)
-LED mounts vertically on top right
-Electrolytic caps C4 and C5 mount vertically
-Screw Terminals attached to Bottom















ASSEMBLY:
Option 2 & 3:
-3D printed mounting hole standoffs with outer rectangle
-11mm Standoffs (distance between LCD and PCB)
-Male headers on LCD
-Female headers on PCB
-1.6mm PCBs