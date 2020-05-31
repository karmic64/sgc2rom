# sgc2rom
Sega Master System/Game Gear SGC to ROM converter.


## Usage
Usage is nothing more than `sgc2rom sgcfile`. If everything went well, a ROM will be created with the same filename as the SGC with `.sms` or `.gg` appended to it.

When you load the ROM, use the D-Pad to step through songs and any other button to restart the current one. The Reset button may be used to return to the default song.


## Build info
If you want to modify the driver, it's in [wla-z80](https://github.com/vhelin/wla-dx) syntax. Keep it under $400 bytes.

To assemble it:
```
wla-z80 -o drv.o drv.asm
wlalink -b drv.ln drv.bin
```

The method I use of including the driver in the executable might be incompatible with some compilers. If you have any problems, please contact me.