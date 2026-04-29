#include std/std.w;

text _start = {
	rodata str = {str: "Hello, cruel w...\n"; len: $str.len - $str.str};
	!syscall(1, 1, $str.str, str.len);
	!syscall(60, 0);}
