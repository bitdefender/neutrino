#ifndef _BUFFERS_H_
#define _BUFFERS_H_

#define TCOUNT 512
#define TMAX 4092

struct Test {
	unsigned int length;
	unsigned char test[TMAX];
};

extern int testCount;
extern Test tests[TCOUNT];

#endif
