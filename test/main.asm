global _start
section .text
extern print
_start:
sub rsp, 8
sub rsp, 8
mov rax, 0
push rax
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
push QWORD [rsp + 8]
mov rax, 0
push rax
pop rax
pop rbx
cmp rbx, rax
setg al
movzx rax, al
push rax
pop rax
test rax, rax
jz label0
mov rax, 1
push rax
pop rax
mov [rsp + 0], rax
mov rdi, rax
call print
jmp label1
label0:
label1:
push QWORD [rsp + 8]
mov rax, 0
push rax
pop rax
pop rbx
cmp rbx, rax
setl al
movzx rax, al
push rax
pop rax
test rax, rax
jz label2
mov rax, 0
push rax
mov rax, 1
push rax
pop rax
pop rbx
sub rbx, rax
push rbx
pop rax
mov [rsp + 0], rax
mov rdi, rax
call print
jmp label3
label2:
label3:
add rsp, 16
mov rax, 60
xor rdi, rdi
syscall
