main:
    mov r0, 6
    b factorial

factorial:
    mov r1, 1

loop:
    cmp r0, 0
    beq exit

    mul r1, r1, r0

    sub r0, r0, 1
    b loop

exit:
    halt

/* functions */

func:
    mov r3, 4
    ret