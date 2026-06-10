#ifndef DING_EVAL
#define DING_EVAL

struct math_s {
	long val; // either an index of varv or a literal or a register
	long type;
	long regi; // reg result
	long op;
	long scopei;};

struct lexe_s {
	struct math_s* mathv; long mathc;
	struct math_s** mathstackv; long mathstackc;
	long* mathbranchv; long mathbranchc;
	long hanging;};

struct eval_s {
	long val;
	long type;};

extern void plugl(long);
extern void plugvar(long);
extern void plugop(long);
extern struct eval_s eval(long);

extern struct lexe_s lexe;

#endif
