.file "tests/test5/main.txt"
.data
.STR0:
.ascii "Hello world!"
.text
.globl main
.type main, @function
main:
	pushq %rbp
	movq %rsp, %rbp
	movl $45, -4(%rbp)
	leaq -4(%rbp), %rbx
	movq %rbx, -12(%rbp)
	movq -12(%rbp), %rbx
	movl $34, (%rbx)
	leaq -4(%rbp), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $1, %edx
	syscall
	leaq .STR0(%rip), %rbx
	movq %rbx, -20(%rbp)
	movl $1, %eax
	movl $1, %edi
	movq -20(%rbp), %rsi
	movl $12, %edx
	syscall
	movl $1, %eax
	movl $1, %edi
	movq -12(%rbp), %rsi
	movl $1, %edx
	syscall
	movl $10, -4(%rbp)
	movl $1, %eax
	movl $1, %edi
	movq -12(%rbp), %rsi
	movl $1, %edx
	syscall
	movl $60, %eax
	movl $0, %edi
	syscall
	movb $0, %al
	leave
	ret
.size main, .-main
