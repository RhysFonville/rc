	.file	"temp.c"
	.text
	.globl	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	$323445, -8(%rbp)
	movl	$45392, -12(%rbp)
	movw	$1233, -14(%rbp)
	movb	$34, -15(%rbp)
	movl	$1410065407, -20(%rbp)
	movl	-12(%rbp), %eax
	cltq
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	%eax, -12(%rbp)
	movswl	-14(%rbp), %eax
	movl	%eax, -12(%rbp)
	movl	-12(%rbp), %eax
	movw	%ax, -14(%rbp)
	movq	-8(%rbp), %rax
	movw	%ax, -14(%rbp)
	movq	-8(%rbp), %rax
	movb	%al, -15(%rbp)
	movsbl	-15(%rbp), %eax
	movl	%eax, -12(%rbp)
	movl	$0, %eax
	popq	%rbp
	ret
	.size	main, .-main
	.ident	"GCC: (Debian 10.2.1-6) 10.2.1 20210110"
	.section	.note.GNU-stack,"",@progbits
