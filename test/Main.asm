global _start
section .text
extern print
_start:
mov rax, 1
push rax
pop rax
mov [rsp + 0], rax
push QWORD [rsp + 0]
pop rax
test rax, rax
jz label0
mov rax, 0
push rax
pop rax
mov [rsp + 0], rax
call print
jmp label1
label0:
mov rax, 1
push rax
pop rax
mov [rsp + 0], rax
call print
label1:
add rsp, 16
mov rax, 60
xor rdi, rdi
syscall
