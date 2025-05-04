ifdef COMSPEC
DOTEXE:=.exe
else
DOTEXE:=
endif

CFLAGS:=-s -Ofast -Wall

sgc2rom$(DOTEXE): sgc2rom.c drv.bin
	$(CC) $(CFLAGS) -o $@ $<

drv.bin: drv.ln drv.o
	wlalink $< $@

drv.o: drv.asm
	wla-z80 -o $@ $<