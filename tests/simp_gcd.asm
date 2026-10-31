main:
    mov r0, 12
    mov r1, 24

    call get_gcd
exit:
    halt

get_gcd:
    /* spill registers */
    sub sp, sp, 12
    st ra, [sp]
    st r0, 4[sp]
    st r1, 8[sp]

    cmp r0, r1
    bgt set_min

    /* r2 = min(r0, r1) */
    mov r2, r0
set_min:
    mov r2, r1

gcd_loop:   /* while(true) */
    mod r8, r0, r2
    mod r9, r1, r2

    cmp r8, 0       /* r0 % r2 == 0 */
    beq pass1

    sub r2, r2, 1   /* decrement counter */
    cmp r2, 0
    bgt gcd_loop

gcd_ret:
    /* restore registers and return */
    ld ra, [sp]
    ld r0, 4[sp]
    ld r1, 8[sp]
    add sp, sp, 12

    ret

pass1:
    cmp r9, 0       /* r1 % r2 == 0 */
    beq pass2
    sub r2, r2, 1   /* decrement counter */
    b gcd_loop

pass2:
    /* r2 is the greatest common factor */
    b gcd_ret