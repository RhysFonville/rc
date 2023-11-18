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
	
	// l = i
	// (i to l)
	movl	-12(%rbp), %eax
	cltq
	movq	%rax, -8(%rbp)
	
	// i = l
	// (l to i)
	movq	-8(%rbp), %rax
	movl	%eax, -12(%rbp)
	
	// l = s
	// (s to l)
	movswq	-14(%rbp), %rax
	movq	%rax, -8(%rbp)
	
	// s = l
	// (l to s)
	movq	-8(%rbp), %rax
	movw	%ax, -14(%rbp)
	
	// l = c
	// (c to l)
	movsbq	-15(%rbp), %rax
	movq	%rax, -8(%rbp)
	
	// c = l
	// (l to c)
	movq	-8(%rbp), %rax
	movb	%al, -15(%rbp)
	
	
	// i = s
	// (s to i)
	movswl	-14(%rbp), %eax
	movl	%eax, -12(%rbp)
	
	// s = i
	// (i to s)
	movl	-12(%rbp), %eax
	movw	%ax, -14(%rbp)
	
	// i = c
	// (c to i)
	movsbl	-15(%rbp), %eax
	movl	%eax, -12(%rbp)
	
	// c = i
	// (i to c)
	movl	-12(%rbp), %eax
	movb	%al, -15(%rbp)
	
	
	// s = c
	// (c to s)
	movsbw	-15(%rbp), %ax
	movw	%ax, -14(%rbp)
	
	// c = s
	// (s to c)
	movzwl	-14(%rbp), %eax
	movb	%al, -15(%rbp)
	
	movl	$0, %eax
	popq	%rbp
	ret
	.size	main, .-main
	.ident	"GCC: (Debian 10.2.1-6) 10.2.1 20210110"
	.section	.note.GNU-stack,"",@progbits
