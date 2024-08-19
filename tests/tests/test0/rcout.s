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
	movq $9999999999, -19(%rbp)
	movslq -12(%rbp), %rbx
	movq %rbx, -8(%rbp)
	movl -8(%rbp), %ebx
	movl %ebx, -12(%rbp)
	movswq -14(%rbp), %rbx
	movq %rbx, -8(%rbp)
	movw -8(%rbp), %bx
	movw %bx, -14(%rbp)
	movsbq -15(%rbp), %rbx
	movq %rbx, -8(%rbp)
	movb -8(%rbp), %bl
	movb %bl, -15(%rbp)
	movswl -14(%rbp), %ebx
	movl %ebx, -12(%rbp)
	movw -12(%rbp), %bx
	movw %bx, -14(%rbp)
	movsbl -15(%rbp), %ebx
	movl %ebx, -12(%rbp)
	movb -12(%rbp), %bl
	movb %bl, -15(%rbp)
	movsbw -15(%rbp), %bx
	movw %bx, -14(%rbp)
	movb -14(%rbp), %bl
	movb %bl, -15(%rbp)
	movl $60, %eax
	movl $0, %edi
	syscall
	movb $0, %al
	leave
	ret
.size main, .-main
