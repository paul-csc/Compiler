global _start
section .text
_start:
mov rax, 2
push rax
push QWORD [rsp + 0]
pop rax
test rax, rax
jz label0
mov rax, 2
push rax
push QWORD [rsp + 8]
mov rax, 100
push rax
pop rax
pop rbx
add rax, rbx
push rax
pop rax
pop rbx
mul rbx
push rax
pop rax
mov [rsp + 0], rax
label0:
push QWORD [rsp + 0]
add rsp, 8
mov rax, 60
pop rdi
syscall
