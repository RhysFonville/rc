.file	"temp.c"
.text
.globl	babi
.type	babi, @function
babi:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	$42, -4(%rbp)
	leaq	-4(%rbp), %rax
	movl	$1, %edx
	movq	%rax, %rsi
	movl	$1, %edi
	call	write@PLT
	
	movl	-4(%rbp), %eax
	leave
	ret
.size	babi, .-babi
.globl	main
.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movl	$0, %eax
	call	babi
	movl	%eax, -4(%rbp)
	movl	$0, %eax
	leave
	ret
.size	main, .-main
.ident	"GCC: (Debian 10.2.1-6) 10.2.1 20210110"
.section	.note.GNU-stack,"",@progbits
