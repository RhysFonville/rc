.file "tests/test8/main.txt"
.data
.STR1:
.asciz "Your new character is: "
.STR0:
.asciz "Enter a character: "
.text
.globl main
.type main, @function
main:
	pushq %rbp
	movq %rsp, %rbp
	movb $10, -1(%rbp)
	movb $1, -2(%rbp)
	movb $0, -3(%rbp)
	movb $1, -4(%rbp)
	jmp .L0
.L1:
	leaq .STR0(%rip), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $19, %edx
	syscall
	movb $1, %bl
	cmpb %bl, -4(%rbp)
	jne .L2
	movb $67, -3(%rbp)
	movb $0, -4(%rbp)
	jmp .L3
.L2:
	movb $126, -3(%rbp)
.L3:
	movb $126, %bl
	cmpb %bl, -3(%rbp)
	je .L4
	movb $1, %bl
	addb -3(%rbp), %bl
	movb %bl, -3(%rbp)
	leaq .STR1(%rip), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $23, %edx
	syscall
	leaq -3(%rbp), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $1, %edx
	syscall
	leaq -1(%rbp), %rbx
	movl $1, %eax
	movl $1, %edi
	movq %rbx, %rsi
	movl $1, %edx
	syscall
	jmp .L5
.L4:
	movb $0, -2(%rbp)
.L5:
.L0:
	movb $0, %bl
	cmpb %bl, -2(%rbp)
	jne .L1
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
