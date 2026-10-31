main:
    mov r0, 0x800   /* address of first element in array p */
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

    mov r1, 6   /* size of array */

    call isort

exit:
    halt

isort:
    /* spill registers */
    sub sp, sp, 16
    st ra, [sp]
    st r0, 4[sp]
    st r1, 8[sp]

    mov r2, 0   /* counter for 'for' i */
isort_for:  /* do */

    cmp r1, r2
    beq isort_return

    mov r3, r2  /* counter for 'while' j */
    cmp r3, 0

    bgt isort_while

is_while_ext:
    add r2, r2, 1
    b isort_for

isort_while:
    cmp r3, 0
    beq is_while_ext

    /* r10: (p + j) and r11: (p + j - 1) */
    mov r10, r3         /* index in r10  */
    mul r10, r10, 4     /* scale index by 4 */
    add r10, r10, r0    /* add scaled index to base address */

    /* compare *(p + j) and *(p + j - 1)*/
    ld r8, [r10]
    ld r9, -4[r10]
    cmp r8, r9  /* a[j] - a[j-1]  */
    bgt is_while_ext


    /* swap *(p + j) and *(p + j - 1) */
    st r8, -4[r10]
    st r9, [r10]
    sub r3, r3, 1
    b isort_while

isort_return:
    /* restore registers and return */
    ld ra, [sp]
    ld r0, 4[sp]
    ld r1, 8[sp]
    add sp, sp, 16
    ret