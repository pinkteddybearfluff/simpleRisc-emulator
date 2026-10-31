main:
    mov r0, 5   /* value to calculate factorial of */
    mov r1, 1   /* result of factorial of value in r0 */
    call factorial


exit:
    halt

/* functions and other labels*/

factorial:
    /* register spilling */

    sub sp, sp, 8
    st ra, [sp]
    st r0, 4[sp]

    cmp r0, 1   /* if r0 == 1 then return */
    beq return
    mul r1, r1, r0
    sub r0, r0, 1
    call factorial

return:
    /* restore registers */
    ld ra, [sp]
    ld r0, 4[sp]
    add sp, sp, 8
    ret

/*  void factorial(int* num, int* res)  */
/*  {                                   */
/*     if (*num == 1)                   */
/*     {                                */
/*         return;                      */
/*     }                                */
/*     *res = (*res) * (*num);          */
/*     --*num;                          */
/*     factorial(num, res);             */
/*     return;                          */
/* }                                    */
