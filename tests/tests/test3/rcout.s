.file "tests/test3/main.txt"
.data
.globl i
.align 8
.type i, @object
.size i, 4
i:
.word 48
.text
.globl is_true
.type is_true, @function
is_true:
	pushq %rbp
	movq %rsp, %rbp
	movl i(%rip), %ebx
	addl $2, %ebx
	movl %ebx, i(%rip)
	movl i(%rip), %eax
	leave
	ret
.size is_true, .-is_true
.globl main
.type main, @function
main:
	pushq %rbp
	movq %rsp, %rbp
	movl i(%rip), %ebx
	movw %bx, -2(%rbp)
	movl i(%rip), %ebx
	movl %ebx, -6(%rbp)
	movb $2, %bl
	cmpw %bx, -2(%rbp)
	jne .L0
	movl $0, %eax
	call is_true
	movl %eax, -6(%rbp)
.L0:
	leaq -6(%rbp), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $1, %edx
	syscall
	movl i(%rip), %ebx
	cmpw %bx, -2(%rbp)
	jne .L1
	movl $0, %eax
	call is_true
	movl %eax, -6(%rbp)
.L1:
	leaq -6(%rbp), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $1, %edx
	syscall
	movl $10, -6(%rbp)
	leaq -6(%rbp), %rbx
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
