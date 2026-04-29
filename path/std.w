// types
struct str_s: {
	char str;
	long len: 0;}

// inlines
inline syscall(varg): {
	#if (vargw >= 8) {#asm movq 8(varg), %rax;}
	#if (vargw >= 16) {#asm movq 16(varg), %rdi;}
	#if (vargw >= 24) {#asm movq 24(varg), %rsi;}
	#if (vargw >= 32) {#asm movq 32(varg), %rdx;}
	#if (vargw >= 40) {#asm movq 40(varg), %r10;}
	#if (vargw >= 48) {#asm movq 48(varg), %r8;}
	#if (vargw >= 56) {#asm movq 56(varg), %r9;}
	#asm syscall;}
