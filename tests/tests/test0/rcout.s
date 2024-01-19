.file "tests/test0/main.txt"
.data
.text
.globl main
.type main, @function
main:
	pushq %rbp
	movq %rsp, %rbp
	movq $323445, -8(%rbp)
	movl $45392, -12(%rbp)
	movw $1233, -14(%rbp)
	movb $34, -15(%rbp)
	movl $9999999999, -19(%rbp)
	movl -12(%rbp), %eax
	cltq
	movq %rax, -8(%rbp)
	movq -8(%rbp), %rbx
	movl %ebx, -12(%rbp)
	movswq -14(%rbp), %rbx
	movq %rbx, -8(%rbp)
	movq -8(%rbp), %rbx
	movw %bx, -14(%rbp)
	movsbq -15(%rbp), %rbx
	movq %rbx, -8(%rbp)
	movq -8(%rbp), %rbx
	movb %bl, -15(%rbp)
	movswl -14(%rbp), %ebx
	movl %ebx, -12(%rbp)
	movl -12(%rbp), %ebx
	movw %bx, -14(%rbp)
	movsbl -15(%rbp), %ebx
	movl %ebx, -12(%rbp)
	movl -12(%rbp), %ebx
	movb %bl, -15(%rbp)
	movsbw -15(%rbp), %bx
	movw %bx, -14(%rbp)
	movw -14(%rbp), %bx
	movb %bl, -15(%rbp)
	movq $60, %rax
	movq $0, %rdi
	syscall
	movb $0, %al
	leave
	ret
.size main, .-main
