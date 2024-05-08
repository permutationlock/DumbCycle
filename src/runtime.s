.text
.global syscall0
syscall0:
    movq %rdi, %rax
    syscall
    ret
.type syscall0, @function
.size syscall0, .-syscall0

.global syscall1
syscall1:
    movq %rdi, %rax
    movq %rsi, %rdi
    syscall
    ret
.type syscall1, @function
.size syscall1, .-syscall1

.global syscall2
syscall2:
    movq %rdi, %rax
    movq %rsi, %rdi
    movq %rdx, %rsi
    syscall
    ret
.type syscall2, @function
.size syscall2, .-syscall2

.global syscall3
syscall3:
    movq %rdi, %rax
    movq %rsi, %rdi
    movq %rdx, %rsi
    movq %rcx, %rdx
    syscall
    ret
.type syscall3, @function
.size syscall3, .-syscall3

.global syscall4
syscall4:
    movq %rdi, %rax
    movq %rsi, %rdi
    movq %rdx, %rsi
    movq %rcx, %rdx
    movq %r8, %r10
    syscall
    ret
.type syscall4, @function
.size syscall4, .-syscall4

.global syscall5
syscall5:
    movq %rdi, %rax
    movq %rsi, %rdi
    movq %rdx, %rsi
    movq %rcx, %rdx
    movq %r8, %r10
    movq %r9, %r8
    syscall
    ret
.type syscall5, @function
.size syscall5, .-syscall5

.global syscall6
syscall6:
    movq %r9, %r11
    movq %rdi, %rax
    movq 8(%rsp), %r9
    movq %rsi, %rdi
    movq %rdx, %rsi
    movq %rcx, %rdx
    movq %r8, %r10
    movq %r11, %r8
    syscall
    ret
.type syscall6, @function
.size syscall6, .-syscall6

.extern _cstart
.global _start
_start:
    xor %rbp,%rbp
    mov (%rsp),%rdi
    mov %rsp,%rsi
    add $8,%rsi
    call _cstart
    ud2

.section .note.GNU-stack,"",@progbits
