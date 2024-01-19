.file "tests/test1/main.txt"
.data
.text
.globl main
.type main, @function
main:
	pushq %rbp
	movq %rsp, %rbp
	movb $10, -1(%rbp)
	movl $5, -5(%rbp)
	movb $2, -6(%rbp)
	movq $48, -14(%rbp)
	movw $8, -16(%rbp)
	movb -6(%rbp), %bl
	addb $4, %bl
	movl %ebx, -5(%rbp)
	movl -5(%rbp), %ebx
	addl -14(%rbp), %ebx
	movl %ebx, -5(%rbp)
	leaq -5(%rbp), %rbx
	movb $1, %bl
	addb -6(%rbp), %bl
	movb %bl, %r10b
	subb $2, %r10b
	movq $1, %rax
	movq $1, %rdi
	movq %rbx, %rsi
	movsbq %r10b, %rdx
	syscall
	leaq -1(%rbp), %rbx
	movq $1, %rax
	movq $1, %rdi
	movq %rbx, %rsi
	movq $1, %rdx
	syscall
	movq $60, %rax
	movq $0, %rdi
	syscall
	movb $0, %al
	leave
	ret
.size main, .-main
