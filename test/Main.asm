global _start
section .text
extern print
_start:
mov rax, 3
push rax
mov rax, 4
push rax
mov rax, 5
push rax
pop rax
pop rbx
mul rbx
push rax
pop rax
pop rbx
add rax, rbx
push rax
mov rax, 23
push rax
pop rax
pop rbx
sub rax, rbx
push rax
pop rax
mov [rsp + 0], rax
push QWORD [rsp + 0]
pop rax
test rax, rax
jz label0
mov rax, 2
push rax
push QWORD [rsp + 8]
pop rax
pop rbx
add rax, rbx
push rax
mov rax, 2
push rax
pop rax
pop rbx
mul rbx
push rax
pop rax
mov [rsp + 0], rax
add rsp, 8
call print
jmp label1
label0:
push QWORD [rsp + 0]
mov rax, 1
push rax
pop rax
pop rbx
add rax, rbx
push rax
pop rax
mov [rsp + 0], rax
add rsp, 8
call print
label1:
add rsp, 8
mov rax, 60
xor rdi, rdi
syscall
