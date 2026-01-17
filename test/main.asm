global _start
section .text
extern print
_start:
sub rsp, 8
sub rsp, 8
sub rsp, 8
mov rax, 5
push rax
pop rax
mov [rsp + 16], rax
mov rdi, rax
call print
mov rax, 4
push rax
pop rax
mov [rsp + 8], rax
mov rdi, rax
call print
mov rax, 1
push rax
pop rax
mov [rsp + 0], rax
mov rdi, rax
call print
label0:
push QWORD [rsp + 8]
pop rax
test rax, rax
jz label1
push QWORD [rsp + 0]
push QWORD [rsp + 24]
pop rax
pop rbx
mul rbx
push rax
pop rax
mov [rsp + 0], rax
mov rdi, rax
call print
push QWORD [rsp + 8]
mov rax, 1
push rax
pop rax
pop rbx
sub rbx, rax
push rbx
pop rax
mov [rsp + 8], rax
mov rdi, rax
call print
jmp label0
label1:
push QWORD [rsp + 0]
pop rax
mov [rsp + 0], rax
mov rdi, rax
call print
add rsp, 24
mov rax, 60
xor rdi, rdi
syscall
