.file "tests/test6/main.txt"
.data
.STR2:
.asciz "\n"
.STR1:
.asciz "Cool print function, yo! No newline!!"
.STR0:
.asciz "Hello world!\n"
.globl print_str
.align 8
.type print_str, @object
.size print_str, 8
print_str:
.long 0
.text
.globl print
.type print, @function
print:
	pushq %rbp
	movq %rsp, %rbp
	movq print_str(%rip), %rbx
	movb $0, %r10b
	cmpb %r10b, (%rbx)
	je .L0
	movl $1, %eax
	movl $1, %edi
	movq print_str(%rip), %rsi
	movl $1, %edx
	syscall
	movq print_str(%rip), %rbx
	addq $1, %rbx
	movq %rbx, print_str(%rip)
	movl $0, %eax
	call print
.L0:
	movb $0, %al
	leave
	ret
.size print, .-print
.globl main
.type main, @function
main:
	pushq %rbp
	movq %rsp, %rbp
	leaq .STR0(%rip), %rbx
	movq %rbx, print_str(%rip)
	movl $0, %eax
	call print
	leaq .STR1(%rip), %rbx
	movq %rbx, print_str(%rip)
	movl $0, %eax
	call print
	leaq .STR2(%rip), %rbx
	movq %rbx, print_str(%rip)
	movl $0, %eax
	call print
	movl $60, %eax
	movl $0, %edi
	syscall
	movb $0, %al
	leave
	ret
.size main, .-main
