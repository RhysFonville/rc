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
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $1, %edx
	syscall
	movb $48, %bl
	cmpl %ebx, -4(%rbp)
	jg .L2
	movl -4(%rbp), %ebx
	addl $5, %ebx
	movl %ebx, -4(%rbp)
	jmp .L3
.L2:
	movl $15, -4(%rbp)
	movl -4(%rbp), %ebx
	subl $5, %ebx
	movl %ebx, -4(%rbp)
.L3:
	leaq -4(%rbp), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $1, %edx
	syscall
	movl $10, -4(%rbp)
	leaq -4(%rbp), %rbx
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
