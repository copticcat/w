.section .data
a:
	.quad 30
b:
	.quad 25
.section .text
.global _start
_start:
	// 10 + 10
	xorq %rax, %rax
	movq $20, %rax

	// a + 10
	xorq %rax, %rax
	movq a, %rax
	addq $10, %rax
	
	// a + b
	xorq %rax, %rax
	movq a, %rax
	addq b, %rax
