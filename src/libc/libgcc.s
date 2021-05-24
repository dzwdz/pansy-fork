# https://wiki.osdev.org/Libgcc
# Yes - I shouldn't write this by hand. However I wasn't able to get gcc to
# actually link against libgcc, so here we are...


.text
.globl __udivmodti4
__udivmodti4:
# rdi - dividend
# rdx - divisor
# r8  - pointer to the remainder
# i have no clue why the calling convention is weird like that

    test %rdx, %rdx # divisor == 0 ?
    jz  .div_by_0

    mov %rdx, %rcx  # move the divisor to RCX
                    # preparing the dividend
    xor %rdx, %rdx  #   zero out RDX (high)
    mov %rdi, %rax  #   copy the actual dividend 
    div %rcx
    mov %rdx, (%r8)
    ret

.div_by_0:
    mov %rdx, %rax  # rdx = 0, we use it to zero out both results
    mov %rdx, (%r8)
    ret
