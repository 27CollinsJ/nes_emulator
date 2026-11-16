

; Syscall custom instruction
.macro cal syscall_id
    .byte $FF
    .byte syscall_id
.endmacro

SYS_PRINT_A = $01
SYS_PRINT_X = $02
SYS_PRINT_Y = $03




.segment "CODE"

.proc main
lda #$ff
cal SYS_PRINT_A
.endproc

.segment "VECTORS"
    .addr 0
    .addr main
    .addr 0