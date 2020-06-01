/* sgc2rom - written by <karmic.c64@gmail.com> */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define lo(i) (i&0xff)
#define hi(i) ((i&0xff00)>>8)

#define get16(p) (*(p)) | ((*(p+1)) << 8)

const uint8_t defaultmappercfg[] = {0,0,1,2};

extern uint8_t driver[];
extern uint8_t driverend[];

asm("driver:  .incbin \"drv.bin\"\n\t"
    "driverend:\n\t"
    );

void write16(uint8_t *p, uint16_t i)
{
    *p = lo(i);
    *(p+1) = hi(i);
}

int main(int argc, char *argv[])
{
    const uint16_t driversize = driverend-driver;
    
    if (argc < 2)
    {
        puts("sgc2rom conversion utility by <karmic.c64@gmail.com>");
        puts("Usage: sgc2rom [sgcfile]...");
        return EXIT_FAILURE;
    }
    
    for (int arg = 1; arg < argc; arg++)
    {
        if (arg > 1) putchar('\n');
        printf("Reading %s\n", argv[arg]);
        
        FILE* infile = fopen(argv[arg], "rb");
        if (!infile)
        {
            perror("Could not open file");
            continue;
        }
        fseek(infile, 0, SEEK_END);
        size_t insize = ftell(infile);
        if (insize < 0xa1)
        {
            fclose(infile);
            puts("File too short to be an SGC");
            continue;
        }
        uint8_t *inbuf = malloc(insize);
        rewind(infile);
        fread(inbuf, 1, insize, infile);
        fclose(infile);
        
        if (memcmp(inbuf, "SGC\x1a", sizeof("SGC\x1a")-1))
        {
            puts("Not an SGC");
            continue;
        }
        int errcnt = 0;
        if (*(inbuf+4) != 1)
        {
            puts("Unknown SGC version number");
            errcnt++;
        }
        uint8_t systype = *(inbuf+0x28);
        if (systype == 2)
        {
            puts("Colecovision SGCs are not supported");
            errcnt++;
        }
        else if (systype > 2)
        {
            puts("Unknown system type");
            errcnt++;
        }
        if (errcnt)
            continue;
        
        uint16_t load = get16(inbuf+8);
        uint32_t end = load + (insize - 0xa0 - 1);
        printf("SGC load range: $%04X-$%04X\n", load, end);
        if (load < 0x0400)
        {
            puts("Illegal load address");
            continue;
        }
        uint16_t init = get16(inbuf+10);
        uint16_t play = get16(inbuf+12);
        printf("SGC init/play: $%04X/$%04X\n", init, play);
        if (init < load || init > end || init >= 0xc000)
        {
            puts("Illegal init address");
            continue;
        }
        uint16_t sp = get16(inbuf+14);
        printf("SGC initial stack pointer: $%04X\n", sp);
        if (sp < 0xc002 || (sp > 0xdff0 && sp < 0xe002) || (sp > 0xfff0))
        {
            puts("Illegal stack pointer");
            continue;
        }
        uint16_t rst[7];
        printf("SGC RST pointers: ");
        for (int i = 0; i < 7; i++)
        {
            rst[i] = get16(inbuf+0x12+(i*2));
            printf("$%04X", rst[i]);
            if (i < 6)
                putchar('/');
        }
        putchar('\n');
        uint8_t mappercfg[4];
        memcpy(mappercfg, inbuf+0x20, 4);
        printf("Default mapper config: $%02X/$%02X/$%02X/$%02X\n",
            mappercfg[0], mappercfg[1], mappercfg[2], mappercfg[3]);
        uint8_t startsong = *(inbuf+0x24);
        uint8_t totalsongs = *(inbuf+0x25);
        uint8_t firstsfx = *(inbuf+0x26);
        uint8_t lastsfx = *(inbuf+0x27);
        printf("Total/default song, sfx range: %u/%u, %u-%u\n",
            totalsongs, startsong, firstsfx, lastsfx);
        if (!totalsongs)
        {
            puts("Illegal amount of songs");
            continue;
        }
        if (startsong >= totalsongs)
        {
            puts("Illegal default song");
            continue;
        }
        if (firstsfx < totalsongs || lastsfx < firstsfx)
        { /* if the sfx range is invalid, assume no sfx */
            firstsfx = 0xff;
        }
        
        size_t outsize = (end < 0x1ff0) ? 0x2000 : ((end & 0xffffc000) + 0x4000);
        /* a lot of emulators won't emulate a mapper if the rom is 48k or less, so
           if the default configuration is nonstandard we need to bloat the rom */
        if (memcmp(defaultmappercfg, mappercfg, 4) && outsize < 0x10000)
            outsize = 0x10000;
        uint8_t *outbuf = malloc(outsize);
        memcpy(outbuf+load, inbuf+0xa0, insize - 0xa0);
        free(inbuf);
        memcpy(outbuf, driver, driversize);
        write16(outbuf+2, sp);
        for (int i = 0; i < 7; i++)
            write16(outbuf+9+(i*8), rst[i]);
        write16(outbuf+0x0c, init);
        write16(outbuf+0x14, play);
        memcpy(outbuf+0x1b, mappercfg, 4);
        *(outbuf+0x23) = startsong;
        *(outbuf+0x24) = totalsongs;
        *(outbuf+0x25) = firstsfx-totalsongs;
        *(outbuf+0x26) = totalsongs + ((firstsfx == 0xff) ? 0 : (lastsfx-firstsfx + 1));
        
        uint8_t *header = outbuf;
        if (outsize == 0x2000)
        {
            header += 0x1ff0;
            *(header + 0xf) = 0x4a;
        }
        else if (outsize == 0x4000)
        {
            header += 0x3ff0;
            *(header + 0xf) = 0x4b;
        }
        else
        {
            header += 0x7ff0;
            *(header + 0xf) = 0x4c;
        }
        /* invalidate any SDSC header */
        if (!memcmp(header-0x10, "SDSC", 4)) *(header-0x10) = 's';
        memcpy(header, "TMR SEGA", sizeof("TMR SEGA")-1);
        uint16_t checksum = 0;
        for (uint8_t *p = outbuf; p < header; p++)
            checksum += *p;
        write16(header+10, checksum);
        
        
        FILE* outfile = fopen(strcat(argv[arg], systype ? ".gg" : ".sms"), "wb");
        fwrite(outbuf, 1, outsize, outfile);
        fclose(outfile);
    }
    
}

