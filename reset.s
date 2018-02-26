; startup code for cc65/ca65
; author: Robert Simari
; data: 2/22/18
; all credit for uncompressing RLE files goes to @Shiru

	.import _main
	.export __STARTUP__:absolute=1
	.export _WaitFrame, _UnRLE
	.exportzp _Frame_Number, _InputPort1, _InputPort1Prev, _InputPort2, _InputPort2Prev  ; export zeropage variables

	; variables created by linker (see nes.cfg)
	.import __STACK_START__, __STACK_SIZE__
	.import __OAM_LOAD__ ; from the define = yes in nes.cfg
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
INPUT         = $4016
INPUT_1       = $4016
INPUT_2       = $4017

.segment "ZEROPAGE"

; put zeropage variables here
_Frame_Number:   .res 1 ; initialize frame number to 1
_Frame_Done:     .res 1

_InputPort1:     .res 1 ; used for reading from the controller
_InputPort1Prev: .res 1
_InputPort2:     .res 1
_InputPort2Prev: .res 1

temp1:           .res 1
temp2:           .res 1

RLE_LOW:         .res 1
RLE_HIGH:        .res 1
RLE_TAG:         .res 1
RLE_BYTE:        .res 1


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
	.res 5, 0                ; set the rest of the 5 bytes to 0, we dont need them

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

vblank_wait_1:
	; first of two vblank waits to ensure that the PPU has started and stabilized
	bit PPU_STATUS
	bpl vblank_wait_1 ; branch if plus

	stx APU_STATUS     ; sets APU_STATUS to 0 which disables music

	txa                ; put the value from x into accumulator, which is $00
clear_ram:
	; puts $00 in all RAM addresses
    sta $00, x
    sta $0100, x
    sta $0300, x ; skip $0200 because thats where OAM data goes
    sta $0400, x ; $0200 should be initialized to $ef-$ff or else
    sta $0500, x ; the sprites will be set at location (0,0)
    sta $0600, x ; ,x means indexed by x so it will go from $00 to $ff
    sta $0700, x ; setting the values to $00 which is the value of the accumulator

    inx ; increment x value

    bne clear_ram ; will jump back to clear_ram if Z in process register is 0
                   ; Z is the result of the last operation, in this case it will
                   ; run $ff times

    lda #$ef       ; loads $ef into the accumulator
clear_oam:
	sta __OAM_LOAD__, x   ; stores the accumulator value at $0200 which is $ef
				          ; indexed by x which is $00 from the last loop
	inx                   ; increment x
	inx
	inx
	inx
	bne clear_oam ; set every 4 bytes to $ef, these are the values for the sprites locations


vblank_wait_2:
	; again make sure PPU has stabilized
	bit PPU_STATUS
	bpl vblank_wait_2

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


_WaitFrame:
	inc _Frame_Done
@loop:
	lda _Frame_Done ; spins until _Frame_Done is set in the nmi handler
	bne @loop

	jsr GetInput    ; read from input
	rts

; updates values read from controller
; for more info see http://wiki.nesdev.com/w/index.php/Controller_Reading
GetInput:
	lda _InputPort1      ; save previous state of controller
	sta _InputPort1Prev 
    lda _InputPort2
    sta _InputPort2Prev

    jsr ReadInput
@conflict:
    lda _InputPort1
    sta temp1
    lda _InputPort2
    sta temp2

    ; get second reading and compare, this is because of a apu glitch
    jsr ReadInput
    lda _InputPort1
    cmp temp1
    bne @conflict
    lda _InputPort2
    cmp temp2
    bne @conflict

    rts

; reads from the controller
ReadInput:
	ldx #$01     ; load value at $01 into x, this "strobes" the controller which gets the controller in sync
    stx INPUT    ; store it at $4016
    dex          
    stx INPUT

    ldy #08         ; loop over all 8 buttons
@loop:
    lda INPUT_1     ; read button state
    and #$03        ; mask lowest 2 bits
    cmp #$01        ; set carry bit to button state
    rol _InputPort1 ; rotate carry bit into button var
    lda INPUT_2     ; repeat for second controller
    and #$03
    cmp #$01
    rol _InputPort2
    dey
    bne @loop

    rts

; code that uncompresses .rle background files
; written by @Shiru and posted on nesdev boards in 2010
; code was provided with NES Screen Tool application
_UnRLE:
	tay
	stx <RLE_HIGH
	lda #0
	sta <RLE_LOW

	lda (RLE_LOW),y
	sta <RLE_TAG
	iny
	bne @1
	inc <RLE_HIGH
@1:
	lda (RLE_LOW),y
	iny
	bne @11
	inc <RLE_HIGH
@11:
	cmp <RLE_TAG
	beq @2
	sta $2007
	sta <RLE_BYTE
	bne @1
@2:
	lda (RLE_LOW),y
	beq @4
	iny
	bne @21
	inc <RLE_HIGH
@21:
	tax
	lda <RLE_BYTE
@3:
	sta $2007
	dex
	bne @3
	beq @1
@4:
	rts


; code for what to do on interrupts
nmi:
    pha ; save registers to stack
    txa
    pha
    tya
    pha

    lda #0               ; load 0 into accumulator
    sta OAM_ADDRESS      ; store 0 at OAM_ADDRESS
    lda #>(__OAM_LOAD__) ; load where the OAM is located
    sta OAM_DMA          ; store it at OAM_DMA
    					 ; this writes sprite data into OAM via DMA
    					 ; Direct Memory Access makes writing sprites faster
    					 ; this saves us precious vblank cycles

	inc _Frame_Number ; keeps count of how many frames have passed
	
	lda #0
	sta _Frame_Done ; set frame done to 0 (see _WaitFrame)

    pla  ; restore registers and return
    tay
    pla
    tax
    pla

	rti
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