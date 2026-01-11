section .text
global _start

extern print

_start:
    mov rax, 123456
    call print

    mov rax, 0
    call print

    ; exit
    mov rax, 60
    xor rdi, rdi
    syscall
