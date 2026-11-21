#ifndef _RB_H
#define _RB_H

#include <stddef.h>

#define RB_INIT(r) \
	((r)->h = (r)->t = 0)

#define RB_EMPTY(r) \
	((r)->h == (r)->t)

#define RB_FULL(r) \
	(((r)->t + 1) % (r)->cap == (r)->h)

#define RB_SIZE(r) \
	(((r)->t + (r)->cap - (r)->h) % (r)->cap)

#define RB_PUSH(r, value) ({ \
	int _ok = !RB_FULL(r); \
	if (_ok) { \
		(r)->buf[(r)->t] = (value); \
		(r)->t = ((r)->t + 1) % (r)->cap; \
	} \
	_ok; \
})

#define RB_POP(r, out) ({ \
	int _ok = !RB_EMPTY(r); \
	if (_ok) { \
		*(out) = (r)->buf[(r)->h]; \
		(r)->h = ((r)->h + 1) % (r)->cap; \
	} \
	_ok; \
})

#define RB_PEEK(r) \
	((r)->buf[(r)->h])

#endif /* _RB_H */
