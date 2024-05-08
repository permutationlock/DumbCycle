.text
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
