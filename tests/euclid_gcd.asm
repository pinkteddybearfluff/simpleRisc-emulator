main:
    mov r0, 16
    mov r1, 12

    call get_gcd

exit:
    halt

get_gcd:
    /* register spilling */
    sub sp, sp, 12
    st ra, [sp]
    st r0, 4[sp]
    st r1, 8[sp]

    cmp r0, r1
    bgt set_min
    call gcd2

set_min:    /* r0 <= r1 */
    mov r0, r1
    ld r1, 4[sp]

    call gcd2

    ld ra, [sp]
    ld r0, 4[sp]
    ld r1, 8[sp]
    add sp, sp, 12
    ret

gcd2:
    /* register spilling */
    sub sp, sp, 12
    st ra, [sp]
    st r0, 4[sp]
    st r1, 8[sp]

    cmp r0, 0
    beq gcd2_ret_r0

    mod r0, r1, r0
    ld r1, 4[sp]
    call gcd2

gcd2_ret:
    ld ra, [sp]
    ld r0, 4[sp]
    ld r1, 8[sp]
    add sp, sp, 12
    ret

gcd2_ret_r0:
    mov r2, r1
    b gcd2_ret