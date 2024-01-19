.file "tests/test4/main.txt"
.data
.text
.globl main
.type main, @function
main:
	pushq %rbp
	movq %rsp, %rbp
	movl $0, -4(%rbp)
	movb $1, %bl
	movb $2, %r10b
	cmpb %r10b, %bl
	jne .L0
	movl $48, -4(%rbp)
	jmp .L1
.L0:
	movl $49, -4(%rbp)
.L1:
	leaq -4(%rbp), %rbx
	movq $1, %rax
	movq $1, %rdi
	movq %rbx, %rsi
	movq $1, %rdx
	syscall
	movb $48, %bl
	cmpl %ebx, -4(%rbp)
	jg .L1
	movl -4(%rbp), %ebx
	addl $5, %ebx
	movl %ebx, -4(%rbp)
	jmp .L2
.L1:
	movl $15, -4(%rbp)
	movl -4(%rbp), %ebx
	subl $5, %ebx
	movl %ebx, -4(%rbp)
.L2:
	leaq -4(%rbp), %rbx
	movq $1, %rax
	movq $1, %rdi
	movq %rbx, %rsi
	movq $1, %rdx
	syscall
	movl $10, -4(%rbp)
	leaq -4(%rbp), %rbx
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
