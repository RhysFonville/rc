section .data
input_prompt: db "Enter two single-digit numbers: "
section .bss
_mul: resw 1
_sub: resw 1
_add: resw 1
n2: resw 1
n1: resw 1
global _start
section .text
_start:
mov rax, 1
mov rdi, 1
mov rsi, input_prompt
mov rdx, 32
syscall
mov rax, 0
mov rdi, 0
mov rsi, n1
mov rdx, 1
syscall
mov rax, 0
mov rdi, 0
mov rsi, n2
mov rdx, 1
syscall
mov %r9, (n1)
mov %r10, (n2)
add %r9, %r10
mov (_add), %r9
mov %r10, (n1)
mov %r11, (n2)
sub %r10, %r11
mov (_sub), %r10
mov %r11, (n1)
mov %r12, (n2)
mul %r11
mov (_mul), %r11
mov rax, 1
mov rdi, 1
mov rsi, _add
mov rdx, 2
syscall
mov rax, 1
mov rdi, 1
mov rsi, _sub
mov rdx, 2
syscall
mov rax, 1
mov rdi, 1
mov rsi, _mul
mov rdx, 2
syscall
mov rax, 60
mov rdi, 0
syscall
