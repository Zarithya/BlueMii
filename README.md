# BlueMii

![Tobias](https://media1.tenor.com/m/_Donca-oV40AAAAC/tobias-funke-david-cross.gif)

BlueMii is a port of [Fullmetal5's BlueBomb exploit](https://github.com/Fullmetal5/bluebomb) for Broadcom's Bluetooth stack used in the Nintendo Wii *to* the Nintendo Wii. This means you can use the Bluetooth capabilities of one Wii / Wii mini / Wii U (vWii) / Dolphin to install homebrew on another Wii/Wii mini. This probably isn't useful to most people, but it was fun to work on.

The stage1 payload is taken from [Aep's fork](https://github.com/Aeplet/bb-stage1-sd), which adds SD card support.

## How to Use

You will need either an already-hacked Wii / Wii mini / Wii U (vWii) console **or** a copy of Dolphin **with working Bluetooth passthrough**.

1. Copy the `apps` folder from the BlueMii `.zip` to the root of your SD card or USB drive and insert it into your *already-hacked* source console.
2. Copy a `boot.elf` of your choosing (usually the [Hackmii Installer](https://bootmii.org/download/)) to the root of another SD card or USB drive and insert it into your *target* console.
3. Power on both consoles.
4. On your *source* console, open the `Homebrew Channel` and load the `BlueMii` app.
5. On your *target* console, go to `Wii Settings` and take note of the system version in the upper-right hand corner of the screen.
6. In the BlueMii app, using the d-pad left and right buttons, choose the option that corresponds to your *target's* System Menu version (the Wii mini has its own separate options, so use those).
7. Power off the *target* console, then turn it back on *without connecting any Wii Remotes*.
8. In the BlueMii app, press the A button to confirm your selection. This will power off any Wii Remotes connected to the *source* console and should display `Waiting to accept...` on the screen.
9. On the *target* console, press the SYNC button repeatedly until the screen displays `Got connection handle`. From here on, the process is automatic, so sit back and watch a Wii / Wii U hack another Wii!

## Troubleshooting
**Q: I'm pressing the SYNC button on my target console but BlueMii only says "Waiting to accept..."!**    
A: BlueMii can't find your other Wii. Try moving the consoles closer to one another ~~and make them kiss~~.

**Q: BlueMii is stuck on "Awaiting response from stage0"!**    
A: Make sure the system menu version and region are correct for your *target* console and that no Wii Remotes are connected.

**Q: BlueMii is stuck on "Uploading payload..."/My target console is frozen!**    
A: Press the SYNC button on your source console to cancel the exploit, then reboot your target console and try again.


