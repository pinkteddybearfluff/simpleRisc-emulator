main:
    mov r0, 0x800
    mov r1, 5
    st r1, [r0]
    mov r1, 16
    st r1, 4[r0]
    mov r1, 1
    st r1, 8[r0]
    mov r1, 6
    st r1, 12[r0]
    mov r1, 4
    st r1, 16[r0]
    mov r1, 15
    st r1, 20[r0]

    call isort
exit:
    halt

isort:
    ret