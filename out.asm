global _start
section .text
_start:
push rbp
mov rbp, rsp
mov rax, 2
push rax
mov rax, QWORD [rbp + 0]
push rax
mov rax, 1
pop rbx
sub rbx, rax
mov rax, rbx
mov rax, 60
pop rdi
syscall
mov rax, 60
mov rdi, 0
syscall
