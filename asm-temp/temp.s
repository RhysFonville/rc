	.file	"temp.c"
	.text
	.globl	babi
	.type	babi, @function
babi:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
	movb	$42, -1(%rbp)
	leaq	-1(%rbp), %rax
	movl	$1, %edx
	movq	%rax, %rsi
	movl	$1, %edi
	call	write@PLT
	movzbl	-1(%rbp), %eax
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
	movb	%al, -1(%rbp)
	movl	$0, %eax
	leave
	ret
	.size	main, .-main
	.ident	"GCC: (Debian 10.2.1-6) 10.2.1 20210110"
	.section	.note.GNU-stack,"",@progbits
