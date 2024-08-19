.file "tests/test9/main.txt"
.data
.text
.globl main
.type main, @function
main:
	pushq %rbp
	movq %rsp, %rbp
	movb $42, -1(%rbp)
	movb $10, -2(%rbp)
	movl $1, -6(%rbp)
	jmp .L0
.L1:
	movl $0, -10(%rbp)
	jmp .L2
.L3:
	leaq -1(%rbp), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $1, %edx
	syscall
	movl -10(%rbp), %ebx
	addl $1, %ebx
	movl %ebx, -10(%rbp)
.L2:
	cmpl -10(%rbp), -6(%rbp)
	jl .L3
	leaq -2(%rbp), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $1, %edx
	syscall
	movl -6(%rbp), %ebx
	addl $1, %ebx
	movl %ebx, -6(%rbp)
.L0:
	cmpl -6(%rbp), %ebx
	jl .L1
	jmp .L4
.L5:
	movl $0, -14(%rbp)
	jmp .L6
.L7:
	leaq -1(%rbp), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $1, %edx
	syscall
	movl -14(%rbp), %ebx
	addl $1, %ebx
	movl %ebx, -14(%rbp)
.L6:
	cmpl -14(%rbp), -6(%rbp)
	jl .L7
	leaq -2(%rbp), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $1, %edx
	syscall
	movl -6(%rbp), %ebx
	subl $1, %ebx
	movl %ebx, -6(%rbp)
.L4:
	cmpl -6(%rbp), %ebx
	jg .L5
	movl $60, %eax
	movl $0, %edi
	syscall
	movb $0, %al
	leave
	ret
.size main, .-main
