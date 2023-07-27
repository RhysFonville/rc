.file "code.txt"
.text
.globl print
.type print, @function
print:
push %rbp
mov %rsp, %rbp
sub $16, %rsp
mov $42, -4(%rbp)
lea -4(%rbp), %rbx
mov $1, %rax
mov $1, %rdi
mov %rbx, %rsi
mov $1, %rdx
syscall
mov -4(%rbp), %rax
leave
ret
.size print, .-print
.globl _start
.type main, @function
main:
push %rbp
mov %rsp, %rbp
sub $16, %rsp
mov $0, %rax
call print
mov %rax, -4(%rbp)
mov $60, %rax
mov $0, %rdi
syscall
mov 0, %rax
leave
ret
.size main, .-main
