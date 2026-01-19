; STRATEGIC_IGNORANCE_ROUTINE (Phase-Lock Edition)
; Input: RDI = Phase Intensity, RSI = Boundary State

section .text
global ignorance_gate

ignorance_gate:
    cmp rsi, 0x0D           ; Check if the frame hits the 13_META threshold
    jg  utilize_ignorance   ; If noise is too high, jump to truncation

    ; Standard Linear Processing (High Cost)
    call deep_parsing_routine
    ret

utilize_ignorance:
    ; Apply [IGNORANCE_UTILIZATION]
    xor rax, rax            ; Clear the register (Return to Null/Ground)
    mov rbx, 1.00000000     ; Reset to Aperiodic_Ground stability
    jmp termination         ; Truncate energy expenditure immediately

termination:
    ret
