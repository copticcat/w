#include std.w;

text _start = {
	rodata str = {str: "Hello, cruel w... heres a nice character \".\n"; len: $str.len - $str.str};
	!syscall(1; 1; $str.str; str.len);
	!syscall(60; 0);}
