section .data
input_prompt: db "Enter a character: "
output_prompt: db "Your new character is: "
section .bss
c: resb 1
global _start
section .text
_start:
mov rax, 1
mov rdi, 1
mov rsi, input_prompt
mov rdx, 19
syscall
mov rax, 0
mov rdi, 0
mov rsi, c
mov rdx, 1
syscall
mov rbx, [c]
mov rcx, 1
add rbx, rcx
mov [c], rbx
mov rax, 1
mov rdi, 1
mov rsi, output_prompt
mov rdx, 23
syscall
mov rax, 1
mov rdi, 1
mov rsi, c
mov rdx, 1
syscall
mov rax, 60
mov rdi, 0
syscall
