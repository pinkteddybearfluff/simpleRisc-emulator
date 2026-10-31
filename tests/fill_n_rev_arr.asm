main:
    mov r0, 0x800   /* pointer to first element of array p */
    mov r1, 10  /* size of array n */
    call fill_array

    call reverse_array


exit:
    halt

/* functions and loops */

fill_array:
    /* spill registers onto stack */
    sub sp, sp, 12
    st r0, [sp]
    st r1, 4[sp]
    st ra, 8[sp]

fa_loop:            /* do */
    st r1, [r0]
    sub r1, r1, 1
    add r0, r0, 4
    cmp r1, 0
    bgt fa_loop     /* while( r1 > 0 )  */

    /* restore registers */
    ld r0, [sp]
    ld r1, 4[sp]
    ld ra, 8[sp]
    add sp, sp, 12
    ret

reverse_array:
    sub sp, sp, 16
    st r0, [sp]
    st r1, 4[sp]
    st r2, 8[sp]
    st ra, 12[sp]

    mul r1, r1, 4
    add r2, r0, r1  /* pointer to *(p+n) = q  */
    sub r2, r2, 4

ra_loop:  /* do  */
    /* swap *p and *q  */
    ld r8, [r0]
    ld r9, [r2]
    st r8, [r2]
    st r9, [r0]

    add r0, r0, 4
    sub r2, r2, 4

    cmp r2, r0
    bgt ra_loop   /* while (q > p) */

    /* restore registers  */
    /* ld r0, [sp]  */
    ld r1, 4[sp]
    /* ld r2, 8[sp]  */
    ld ra, 12[sp]
    add sp, sp, 16
    ret