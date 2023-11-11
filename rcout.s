.file "code.txt"
.data
.globl notfive
.align 0
.type notfive, @object
notfive:
.asciz "Your number doesn't equal 5.\n"
.globl five
.align 0
.type five, @object
five:
.asciz "Your number equals 5.\n"
.globl s
.align 0
.type s, @object
s:
.asciz "Enter a single-digit number: "
.text
.globl fiveoutput
.type fiveoutput, @function
fiveoutput:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movq $1, %rax
movq $1, %rdi
movq $five, %rsi
movq $22, %rdx
syscall
movb $0, %al
leave
ret
.size fiveoutput, .-fiveoutput
.globl notfiveoutput
.type notfiveoutput, @function
notfiveoutput:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movq $1, %rax
movq $1, %rdi
movq $notfive, %rsi
movq $29, %rdx
syscall
movb $0, %al
leave
ret
.size notfiveoutput, .-notfiveoutput
.globl main
.type main, @function
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movb $0, -1(%rbp)
movq $1, %rax
movq $1, %rdi
movq $s, %rsi
movq $29, %rdx
syscall
leaq -1(%rbp), %rbx
movq $0, %rax
movq $0, %rdi
movq %rbx, %rsi
movq $1, %rdx
syscall
movb $53, %al
cmpb %al, -1(%rbp)
jne .IF0
movl $0, %eax
call fiveoutput
.IF0:
movb $53, %al
cmpb %al, -1(%rbp)
je .IF1
movl $0, %eax
call notfiveoutput
.IF1:
movq $60, %rax
movq $0, %rdi
syscall
movb $0, %al
leave
ret
.size main, .-main
