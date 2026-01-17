section .data
    buffer  db 20 dup(0)   ; 20 digits
    newline db 0xA         ; newline character

section .text
global print

; print first argument in RDI
print:
    ; save registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi

    mov rax, rdi           ; move argument from RDI to RAX for math
    mov rbx, 10            ; divisor = 10
    lea rsi, [buffer + 20] ; point RSI just past end of buffer
    xor rcx, rcx           ; digit count = 0
    xor r9, r9             ; negative flag = 0

    ; handle negative numbers
    test rax, rax
    jge .convert_start
    neg rax                 ; make number positive
    mov r9, 1               ; mark negative

.convert_start:
    ; special-case zero
    cmp rax, 0
    je .store_zero

.convert_loop:
    xor rdx, rdx            ; clear RDX before DIV
    div rbx                 ; divide RDX:RAX by 10 -> quotient in RAX, remainder in RDX
    add dl, '0'             ; convert remainder to ASCII
    dec rsi
    mov [rsi], dl
    inc rcx
    test rax, rax
    jnz .convert_loop
    jmp .maybe_sign

.store_zero:
    dec rsi
    mov byte [rsi], '0'
    inc rcx

.maybe_sign:
    cmp r9, 0
    je .print_out
    ; prepend '-' if original was negative
    dec rsi
    mov byte [rsi], '-'
    inc rcx

.print_out:
    ; write the number string at RSI, length in RCX
    mov rax, 1          ; sys_write
    mov rdi, 1          ; stdout
    mov rdx, rcx        ; length
    ; RSI already points to start
    syscall

    ; write newline
    mov rax, 1
    mov rdi, 1
    lea rsi, [newline]
    mov rdx, 1
    syscall

    ; restore registers
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    ret
