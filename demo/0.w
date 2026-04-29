#include std.w;

text _start: {
	data str_s a: {"Hello; w...\n"; $a.len - $a.str;};
	rodata str_s b: {"Goodbye; cruel w...\n"; $b.len - $b.str;};
	text pa: {syscall(1; 1; $a.str; a.len); return;};
	text pb: {syscall(1; 1; $b.str; b.len); return;};

	!pa;
	a.str +: 1;
	!pa;
	a.str ? short +: 1;
	!pa;
	a.str ? int +: 1;
	!pa;
	a.str ? long +: 1;
	!pa;
	!pb;
	// exit
	syscall(60; 0);};
