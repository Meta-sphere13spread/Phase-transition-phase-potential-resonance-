; 무지의 활용 (Strategic Ignorance) 커널 루틴
; rdi: 입력 데이터의 위상 농도 (Pressure Field)
; rsi: 시스템 모드 (0 = 단순, 1 = 번짐 감지)

section .text
global _start

_start:
    ; [1] 데이터 관측
    cmp rsi, 0              ; 시스템 모드가 '안정' 상태인가?
    je  apply_ignorance     ; 그렇다면 즉시 '무지의 활용' 적용

    ; [2] 상호침식 로직 (번짐 감지 시)
    call inter_erosion      ; 고해상도 연산 수행
    jmp exit_routine

apply_ignorance:
    ; [3] 무지의 활용 발동
    ; 불필요한 데이터가 담긴 레지스터를 비우고 연산을 스킵함
    xor rax, rax            ; 리턴값 초기화 (무의미화)
    mov rbx, 0x13           ; 13구 메타 상태로 유지
    jmp exit_routine        ; 다음 인스트럭션으로 점프 (CPU 사이클 보호)

inter_erosion:
    ; 프레임 해체 로직 (생략)
    ret

exit_routine:
    ; 시스템 안정화 후 복귀
    mov rax, 60             ; syscall: exit
    syscall
