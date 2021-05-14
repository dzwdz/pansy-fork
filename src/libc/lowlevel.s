.text
.globl _start, _syscall

_start:
/*    When _start gets executed, the stack looks like this:
         0 | argc
         8 | argv
    argc+8 | envp
    
    i'm just moving everything to a place that the main() wrapper knows about
    TODO check the effective uid - i think that it's important
    */

    mov  (%rsp), %rdi
    lea 8(%rsp), %rsi
    mov   $main, %rdx
    call __libc_start_main
    
    mov %rax, %rdi    /* return value of main() */
    mov $60, %rax    /* SYS_exit */
    syscall

_syscall:
    mov %rdi, %rax
    mov %rsi, %rdi
    mov %rdx, %rsi
    mov %rcx, %rdx
    mov %r8, %r10
    mov %r9, %r8
    mov 8(%rsp), %r9
    syscall
    ret
