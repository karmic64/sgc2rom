.memorymap
defaultslot 0
slot 0 0 $400
.endme

.rombankmap
bankstotal 1
banksize $400
banks 1
.endro
            
            
            .bank 0
            .orga 0
            di
            ld sp,0
            in a,($bf)
            jr reset
            
            .orga $08
            jp 0
init:       jp 0
            
            .orga $10
            jp 0
play:       jp 0
            
            .orga $18
            jp 0
mappercfg:  .db 0,0,0,0
            
            .orga $20
            jp 0
startsong:  .db 0
totalsongs: .db 0
sfxdiff:    .db 0 ;first sfx - amount of songs
totaltracks:
            .db 0
            
            .orga $28
            jp 0
psg_mute_tbl:
            .db $9f,$bf,$df,$ff
            
            .orga $30
            jp 0
            
            .orga $38
            jp 0
            
            
            .define tuneid $dffb
            .define prvjoy $dffa
            .define stopped $dff9
            
            
reset:      ld a,$7f
            ld (prvjoy),a
            out ($3f),a
            out ($f2),a
            ld bc,$80bf
            ld a,4
            out (c),a
            out (c),b
            inc b
            ld a,$20
            out (c),a
            out (c),b
            
            im 2 ;sgc uses $38 so can't use im 1
            ld a,2
            ld i,a
                
resetbtn:   ld a,(startsong)
            ld (tuneid),a
            
            jr +
            
            .orga $66
            push af
            ld a,1
            ld (stopped),a
            pop af
            retn
            
+:          
            
clear:      ld hl,mappercfg
            ld de,$fffc
            ld bc,4
            ldir
            
            call clear_sound
            
            ld hl,$c001 ;don't clear $c000 because bios might store $3e here
            ld (hl),0
            ld de,$c002
            ld bc,$1fee
            ldir
            
            xor a
            ld (stopped),a
            
            ld a,(tuneid)
            ld hl,totalsongs
            cp (hl)
            jr c, +
            inc l
            add (hl)
+:          call init
            
            
main:       in a,($bf)
            ei
            halt
            
            call play_sub
            
            ld hl,stopped
            ld a,(hl)
            or a
            jr nz,+
            in a,($38)
            cp a,$38
            jr z,+
            in a,($00)
            or a
            jp m,+
            ld (hl),1
+:
            ld hl,prvjoy
            ld b,(hl)
            in a,($dd)
            and $10
            rlca
            rlca
            ld c,a
            in a,($dc)
            and $3f
            or c
            ld (hl),a
            cpl
            and b
            bit 6,a
            jr nz,resetbtn
            ld b,a
            
            ld hl,tuneid
            ld de,totaltracks
            
            and $09
            jr z,+
            inc (hl)
            ld a,(de)
            cp (hl)
            jr nz,+
            ld (hl),0
+:          
            ld a,$06
            and b
            jr z,++
            ld a,(hl)
            or a
            jr nz,+
            ld a,(de)
            ld (hl),a
+           dec (hl)
++:         
            ld a,b
            or a
            jp nz,clear
            jr main
            



play_sub:
            ld hl,stopped
            ld a,(hl)
            dec a
            jp m,play
            ret nz
            ld (hl),2

clear_sound:
            ld hl,psg_mute_tbl
            ld bc,$047f
            otir
            
            
            ld b,9
-:          ld a,b
            add $0f
            call clear_ym_reg
            ld a,b
            add $1f
            call clear_ym_reg
            djnz -

            ret            

            
clear_ym_reg:
            out ($f0),a
            call ym_delay
            xor a
            out ($f1),a
ym_delay:   push af
            pop af
            ret
            
            
            .orga $0200
            .dsb 257, 3
            
            .orga $0303
            ret
            
            
