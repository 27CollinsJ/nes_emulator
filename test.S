

; Syscall custom instruction
.macro cal syscall_id
    .byte $FF
    .byte syscall_id
.endmacro

SYS_PRINT_A = $01
SYS_PRINT_X = $02
SYS_PRINT_Y = $03
SYS_PRINT_REG = $04
SYS_PRINT_MEM = $05 ; low byte X, high byte Y
SYS_HALT = $06



.segment "CODE"

.proc main
    ; --- 1. Immediate & N/Z Flag Edge Cases ---
    lda #$80
    cal SYS_PRINT_REG   ; Expect: A=$80, N=1, Z=0
    lda #$01
    cal SYS_PRINT_REG   ; Expect: A=$01, N=0, Z=0 (N clears, bit 0 not treated as N)
    lda #$00
    cal SYS_PRINT_REG   ; Expect: A=$00, N=0, Z=1 (Z sets)

    ; --- 2. Implicit (TAX) ---
    lda #$42
    tax
    cal SYS_PRINT_X     ; Expect: X=$42

    ; --- 3. Zero Page & Zero Page, X (including $FF wrap-around) ---
    sta $10             ; $0010 = $42
    ldx #$05
    lda #$99
    sta $10, x          ; $0015 = $99
    ldx #$FF
    lda #$77
    sta $05, x          ; $0004 = $77 (must wrap in ZP, NOT write to $0104)

    ; Verify ZP,X wrap via SYS_PRINT_MEM ($0004 vs $0104)
    ldx #$04
    ldy #$00
    cal SYS_PRINT_MEM   ; Expect: [$0004] = $77
    ldy #$01
    cal SYS_PRINT_MEM   ; Expect: [$0104] = $00 (unchanged)

    ; Read back ZP & ZP,X
    lda $10
    cal SYS_PRINT_A     ; Expect: A=$42
    ldx #$05
    lda $10, x
    cal SYS_PRINT_A     ; Expect: A=$99
    ldx #$FF
    lda $05, x
    cal SYS_PRINT_A     ; Expect: A=$77

    ; --- 4. Absolute & Zero Page, Y ---
    lda #$55
    sta $0014           ; $0014 = $55
    ldy #$04
    ldx $10, y          ; ZP,Y read ($10 + $04 = $0014) -> X=$55
    cal SYS_PRINT_X     ; Expect: X=$55

    ; --- 5. Absolute, X & Absolute, Y (Page Crossing) ---
    lda #$AA
    sta $02F0           ; $02F0 = $AA
    ldx #$20
    lda #$BB
    sta $02F0, x        ; Page cross write -> $0310 = $BB

    ldx #$10
    ldy #$03
    cal SYS_PRINT_MEM   ; Expect: [$0310] = $BB

    ldy #$20
    lda $02F0, y        ; Page cross read -> A=$BB
    cal SYS_PRINT_A     ; Expect: A=$BB

    ; --- 6. Indirect JMP ($xxFF Page-Wrap Hardware Quirk) ---
    ; Set up target at $8000 (or replace #$00/#$80 with low/high bytes of a target label)
    lda #$34
    sta $02FF           ; Low byte at $02FF = $34
    lda #$80
    sta $0200           ; High byte at $0200 = $80 (wrapped page!)
    lda #$99
    sta $0300           ; Trap byte at $0300 (must NOT be read as high byte)

    ; Point ($02FF) to our halt block below (e.g., if placed at $8080, use #$80/#$80)
    ; For a self-contained test without hardcoded labels, swap #$34/#$80 for your target label:
    ;   lda #<target
    ;   sta $02FF
    ;   lda #>target
    ;   sta $0200
    ;   jmp ($02FF)

    cal SYS_HALT

.endproc

.segment "VECTORS"
    .addr 0
    .addr main
    .addr 0