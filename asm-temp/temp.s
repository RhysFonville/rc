	.file	"temp.c"
	.text
	.globl	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	movb	$3, -1(%rbp)
	movb	$3, -2(%rbp)
	movzbl	-1(%rbp), %eax
	cmpb	-2(%rbp), %al
	jne	.L2
	movb	$4, -1(%rbp)
.L2:
	movl	$0, %eax
	popq	%rbp
	ret
	.size	main, .-main
	.ident	"GCC: (Debian 10.2.1-6) 10.2.1 20210110"
	.section	.note.GNU-stack,"",@progbits
