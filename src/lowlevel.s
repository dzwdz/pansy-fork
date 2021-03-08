.text
.globl _start, _syscall

_start: /* todo - pass argv */
	call main
	
	mov %rax, %rdi	/* return value of main() */
	mov $60, %rax	/* SYS_exit */
	syscall

_syscall:
	mov %rdi, %rax
    mov %rsi, %rdi
    mov %rdx, %rsi
    mov %rcx, %rdx
    mov %r8, %r10
    mov %r9, %r8
	syscall
	ret
