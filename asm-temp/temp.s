	.file	"temp.c"
	.text
	.globl	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	$3, -4(%rbp)
	movb	$5, -5(%rbp)
	movsbl	-5(%rbp), %eax
	cmpl	%eax, -4(%rbp)
	jne	.L2
	movl	-4(%rbp), %eax
	movb	%al, -5(%rbp)
.L2:
	movl	$0, %eax
	popq	%rbp
	ret
	.size	main, .-main
	.ident	"GCC: (Debian 10.2.1-6) 10.2.1 20210110"
	.section	.note.GNU-stack,"",@progbits
