.file "tests/test7/main.txt"
.data
.text
.globl main
.type main, @function
main:
	pushq %rbp
	movq %rsp, %rbp
	movl $0, -4(%rbp)
	jmp .L0
.L1:
	movl -4(%rbp), %ebx
	addl $48, %ebx
	movb %bl, -5(%rbp)
	movq $1, %rax
	movq $1, %rdi
	movsbq -5(%rbp), %rsi
	movq $1, %rdx
	syscall
	movl -4(%rbp), %ebx
	addl $1, %ebx
	movl %ebx, -4(%rbp)
.L0:
	movb $11, %bl
	cmpl %ebx, -4(%rbp)
	jl .L1
	movq $60, %rax
	movq $0, %rdi
	syscall
	movb $0, %al
	leave
	ret
.size main, .-main
