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
mov rbx, [n1]
mov rcx, [n2]
add rbx, rcx
mov [_add], rbx
mov rcx, [n1]
mov rsp, [n2]
sub rcx, rsp
mov [_sub], rcx
mov rsp, [n1]
mov rbp, [n2]
mul rsp
mov [_mul], rsp
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
