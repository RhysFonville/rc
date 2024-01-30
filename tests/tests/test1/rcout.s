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
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movsbl %bl, %edx
	syscall
	leaq -1(%rbp), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $1, %edx
	syscall
	movl $60, %eax
	movl $0, %edi
	syscall
	movb $0, %al
	leave
	ret
.size main, .-main
