main:
    mov r3, 10  /* number of terms */
    b fib
exit:
    halt

fib:
    mov r0, 0  /* first term */
    mov r1, 1  /* second term */
    mov r5, 0x800  /* address of first element of array (pointer)*/

fib_loop:
    cmp r3, 1
    beq exit
    mov r9, r0      /* tmp = r0 */
    mov r0, r1      /* r0 = r1 */
    add r1, r1, r9  /* r1 = r1 + tmp  */
    st r1, [r5]     /*  *p = r1  */
    add r5, r5, 4   /* p += 4  */
    sub r3, r3, 1
    b fib_loop