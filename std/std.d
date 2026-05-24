// types
struct str_s{
	char str;
	long len{0;}}

// inlines
inline syscall(__varg; __vargw;){
//	#if (__vargw >= 8) {#asm movq 8(__varg), %rax;}
//	#if (__vargw >= 16) {#asm movq 16(__varg), %rdi;}
//	#if (__vargw >= 24) {#asm movq 24(__varg), %rsi;}
//	#if (__vargw >= 32) {#asm movq 32(__varg), %rdx;}
//	#if (__vargw >= 40) {#asm movq 40(__varg), %r10;}
//	#if (__vargw >= 48) {#asm movq 48(__varg), %r8;}
//	#if (__vargw >= 56) {#asm movq 56(__varg), %r9;}
	#asm syscall;}
