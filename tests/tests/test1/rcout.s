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
	movb %bl, -5(%rbp)
	movl -5(%rbp), %ebx
	addl -14(%rbp), %ebx
	movl %ebx, -5(%rbp)
	movb $1, %bl
	addb -6(%rbp), %bl
	movb %bl, %r10b
	subb $2, %r10b
	leaq -5(%rbp), %r11
	movl $1, %eax
	movl $1, %edi
	movq %r11, %rsi
	movsbl %r10b, %edx
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
