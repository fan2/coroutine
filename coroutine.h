#ifndef C_COROUTINE_H
#define C_COROUTINE_H

#define COROUTINE_DEAD 0		// 终止状态
#define COROUTINE_READY 1		// 初始new状态
#define COROUTINE_RUNNING 2		// resume from READY or SUSPEND
#define COROUTINE_SUSPEND 3		// 中止yield状态

struct schedule;

typedef void (*coroutine_func)(struct schedule *, void *ud);

struct schedule * coroutine_open(void);
void coroutine_close(struct schedule *);

int coroutine_new(struct schedule *, coroutine_func, void *ud);
void coroutine_resume(struct schedule *, int id);
int coroutine_status(struct schedule *, int id);
int coroutine_running(struct schedule *);
void coroutine_yield(struct schedule *);

#endif
