; startup code for cc65/ca65
; author: Robert Simari
; data: 2/22/18

	.import _main
	.export __STARTUP__:absolute=1

	; variables created by linker (see nes.cfg)
	.import __STACK_START__, __STACK_SIZE__
	.include "zeropage.inc"

; definitions that will be helpful to reference later
PPU_CTRL      = $2000
PPU_MASK      = $2001
PPU_STATUS    = $2002
OAM_ADDRESS   = $2003
OAM_DMA       = $4014
APU_DMC       = $4010
APU_STATUS    = $4015
APU_FRAME_CTR = $4017

.segment "ZEROPAGE"

; put zeropage variables here

.segment "HEADER"
	; this is where we define the iNES header that was allocated in nes.cfg
	; all explained at https://wiki.nesdev.com/w/index.php/INES
	.byte $4e, $45, $53, $1a ; this is the default iNES header
	.byte $01                ; defines size of PRG ROM x 16KiB
	.byte $01                ; defines size of CHR ROM x 8KiB
	.byte $00                ; 0 for horizontal mirroring, 1 for vertical
	.byte $00                ; which mapper to use
	.byte $00                ; size of PRG RAM x 8Kib
	.byte $00                ; NTSC
	.byte $00                ; not used
	.res 5, 0                ; set the rest of the 8 bytes to 0, we dont need them

.segment "STARTUP"
	; startup code from nes.cfg

start:
	; initialize RAM and jump to main()
	sei    ; ignore all IRQ's for now
	cld    ; disable decimal mode, because it doesnt exist on NES lol

	ldx #$40
	stx APU_FRAME_CTR ; disables APU frame IRQ's by setting value at address to $40

	ldx #$ff
	txs      ; transfer value from register x to register s
			 ; the s register keeps track of the stack the stack pointer is always at __STACK_START__ + s

	inx              ; increments value at register x, now the value is at $00, from $ff to $00
	stx PPU_CTRL     ; disables NMI for now
	stx PPU_MASK     ; disables rendering for now
	stx APU_DMC      ; disables DMC IRQs

	; this is a sort of sanity check that prevents the NES from being register
	; while the v-blank is true. bit does a bit test on the PPU_STATUS to make sure
	; it is false to start
	bit PPU_STATUS

@vblank_wait_1:
	; first of two vblank waits to ensure that the PPU has started and stabilized
	bit PPU_STATUS
	bpl @vblank_wait_1 ; branch if plus

	stx APU_STATUS     ; sets APU_STATUS to 0 which disables music

	txa                ; put the value from x into accumulator, which is $00
@clear_ram:
	; puts $00 in all RAM addresses
    sta $00, x
    sta $0100, x
    sta $0300, x ; skip $0200 because thats where OAM data goes
    sta $0400, x ; $0200 should be initialized to $ef-$ff or else
    sta $0500, x ; the sprites will be set at location (0,0)
    sta $0600, x ; ,x means indexed by x so it will go from $00 to $ff
    sta $0700, x ; setting the values to $00 which is the value of the accumulator

    inx ; increment x value

    bne @clear_ram ; will jump back to clear_ram if Z in process register is 0
                   ; Z is the result of the last operation, in this case it will
                   ; run $ff times

    lda #$ef       ; loads $ef into the accumulator
@clear_oam:
	sta $0200, x   ; stores the accumulator value at $0200 which is $ef
				   ; indexed by x which is $00 from the last loop
	inx            ; increment x
	inx
	inx
	inx
	bne @clear_oam ; set every 4 bytes to $ef, these are the values for the sprites locations


@vblank_wait_2:
	; again make sure PPU has stabilized
	bit PPU_STATUS
	bpl @vblank_wait_2

	; initialize PPU OAM
	stx OAM_ADDRESS ; store $00 at OAM_ADDRESS
	lda #$02        ; load the value $02 into the accumulator
	sta OAM_DMA     ; store the value at OAM_DMA

	; set the C stack pointer
	lda #<(__STACK_START__ + __STACK_SIZE__)
	sta sp 
    lda #>(__STACK_START__+__STACK_SIZE__)
    sta sp+1

    lda PPU_STATUS ; loaded the value at this register resets the PPU latch

    jmp _main ; jump to our C main()

; code for what to do on interrupts
nmi:
irq:
	rti ; return from interrupt

.segment "RODATA"

.segment "VECTORS"
; set the VECTORS data that is useful to point to our handlers
	.word nmi   ; $fffa vblank
	.word start ; $fffc reset
	.word irq   ; $fffe irq

.segment "CHARS"
; include the CHR ROM data, like backgrounds, sprites
	.incbin "sprites.chr"