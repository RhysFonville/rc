.file "tests/test2/main.txt"
.data
.globl i
.align 8
.type i, @object
.size i, 4
i:
.word 0
.text
.globl add24
.type add24, @function
add24:
	pushq %rbp
	movq %rsp, %rbp
	movl i(%rip), %ebx
	addl $24, %ebx
	movl %ebx, -4(%rbp)
	movl -4(%rbp), %eax
	leave
	ret
.size add24, .-add24
.globl mult2
.type mult2, @function
mult2:
	pushq %rbp
	movq %rsp, %rbp
	movl i(%rip), %ebx
	addl i(%rip), %ebx
	movl %ebx, %eax
	leave
	ret
.size mult2, .-mult2
.globl add48
.type add48, @function
add48:
	pushq %rbp
	movq %rsp, %rbp
	movl $0, %eax
	call add24
	movl %eax, i(%rip)
	movl $0, %eax
	call mult2
	movl %eax, -4(%rbp)
	movl -4(%rbp), %eax
	leave
	ret
.size add48, .-add48
.globl main
.type main, @function
main:
	pushq %rbp
	movq %rsp, %rbp
	movl $0, %eax
	call add48
	movl %eax, i(%rip)
	leaq i(%rip), %rbx
	movq $1, %rax
	movq $1, %rdi
	movq %rbx, %rsi
	movq $1, %rdx
	syscall
	movl $10, i(%rip)
	leaq i(%rip), %rbx
	movq $1, %rax
	movq $1, %rdi
	movq %rbx, %rsi
	movq $1, %rdx
	syscall
	movl $0, %eax
	call mult2
	movq $60, %rax
	movq $0, %rdi
	syscall
	movb $0, %al
	leave
	ret
.size main, .-main
