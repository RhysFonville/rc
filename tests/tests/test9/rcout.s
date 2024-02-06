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
	movl $1, -10(%rbp)
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
	movl -6(%rbp), %ebx
	cmpl %ebx, -10(%rbp)
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
	movb $10, %bl
	cmpl %ebx, -6(%rbp)
	jl .L1
	jmp .L4
.L5:
	movl $0, -10(%rbp)
	jmp .L6
.L7:
	leaq -1(%rbp), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $1, %edx
	syscall
	movl -10(%rbp), %ebx
	addl $1, %ebx
	movl %ebx, -10(%rbp)
.L6:
	movl -6(%rbp), %ebx
	cmpl %ebx, -10(%rbp)
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
	movb $0, %bl
	cmpl %ebx, -6(%rbp)
	jg .L5
	movl $60, %eax
	movl $0, %edi
	syscall
	movb $0, %al
	leave
	ret
.size main, .-main
