.file "tests/test5/main.txt"
.data
.STR0:
.ascii "Hello world!"
.text
.globl main
.type main, @function
main:
	pushq %rbp
	movq %rsp, %rbp
	movl $45, -4(%rbp)
	leaq -4(%rbp), %rbx
	movq %rbx, -12(%rbp)
	movq -12(%rbp), %rbx
	movl $34, (%rbx)
	leaq -4(%rbp), %rbx
	movq $1, %rax
	movq $1, %rdi
	movq %rbx, %rsi
	movq $1, %rdx
	syscall
	leaq .STR0(%rip), %rbx
	movq %rbx, -20(%rbp)
	movq $1, %rax
	movq $1, %rdi
	movq -20(%rbp), %rsi
	movq $12, %rdx
	syscall
	movq $1, %rax
	movq $1, %rdi
	movq -12(%rbp), %rsi
	movq $1, %rdx
	syscall
	movl $10, -4(%rbp)
	movq $1, %rax
	movq $1, %rdi
	movq -12(%rbp), %rsi
	movq $1, %rdx
	syscall
	movq $60, %rax
	movq $0, %rdi
	syscall
	movb $0, %al
	leave
	ret
.size main, .-main
