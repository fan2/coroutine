#include "coroutine.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

#if __APPLE__ && __MACH__
	#include <sys/ucontext.h>
#else 
	#include <ucontext.h>
#endif 

#define STACK_SIZE (1024*1024)
#define DEFAULT_COROUTINE 16

struct coroutine;

// 调度器
struct schedule {
	char stack[STACK_SIZE];    // 所有协程的public stack，栈上空间
	ucontext_t main;           // 主线程的context
	int nco;                   // 当前启用的协程数量
	int cap;                   // 支持的协程数量
	int running;               // 当前正在执行的协程id
	struct coroutine **co;     // 协程对象集: coroutine* []
};

// 协程对象
struct coroutine {
	coroutine_func func;       // 每个协程的回调函数
	void *ud;                  // 每个协程的用户数据
	ucontext_t ctx;            // 每个协程的context
	struct schedule * sch;     // 每个协程从属的调度器
	ptrdiff_t cap;             // 每个协程private stack的最大分配空间
	ptrdiff_t size;            // 每个协程private stack的实际分配空间
	int status;                // 每个协程的当前运行状态
	char *stack;               // 每个协程的private stack
};

struct coroutine * 
_co_new(struct schedule *S , coroutine_func func, void *ud) {
	struct coroutine * co = malloc(sizeof(*co));
	co->func = func;	// 函数
	co->ud = ud;		// 参数
	co->sch = S;		// 调度器
	co->cap = 0;
	co->size = 0;
	co->status = COROUTINE_READY;
	co->stack = NULL;
	return co;
}

void
_co_delete(struct coroutine *co) {
	// 只释放协程自己动态分配的空间, 并注意释放顺序
	free(co->stack);
	free(co);
}

struct schedule * 
coroutine_open(void) {
	struct schedule *S = malloc(sizeof(*S));
	S->nco = 0;
	S->cap = DEFAULT_COROUTINE;
	S->running = -1;
	S->co = malloc(sizeof(struct coroutine *) * S->cap);
	memset(S->co, 0, sizeof(struct coroutine *) * S->cap);
	return S;
}

void 
coroutine_close(struct schedule *S) {
	int i;
	for (i=0;i<S->cap;i++) {
		struct coroutine * co = S->co[i];
		if (co) {
			_co_delete(co);
		}
	}
	// 最后释放调度器
	free(S->co);
	S->co = NULL;
	free(S);
}

int 
coroutine_new(struct schedule *S, coroutine_func func, void *ud) {
	struct coroutine *co = _co_new(S, func , ud);
	if (S->nco >= S->cap) { // 2倍扩容
		int id = S->cap;
		S->co = realloc(S->co, S->cap * 2 * sizeof(struct coroutine *));
		memset(S->co + S->cap , 0 , sizeof(struct coroutine *) * S->cap);
		S->co[S->cap] = co;
		S->cap *= 2;
		++S->nco;
		return id;
	} else {
		int i;
		for (i=0;i<S->cap;i++) {
			int id = (i+S->nco) % S->cap;
			if (S->co[id] == NULL) {     // unuse empty slot
				S->co[id] = co;
				++S->nco;
				return id;              // 返回创建好的协程id
			}
		}
	}
	assert(0);
	return -1;
}

static void
mainfunc(uint32_t low32, uint32_t hi32) {
	// resume param
	uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
	struct schedule *S = (struct schedule *)ptr;
	int id = S->running;
	printf("mainfunc: coroutine id[%d]\n", S->running);
	struct coroutine *C = S->co[id];
	C->func(S,C->ud);	// functional logic unit
	_co_delete(C);
	S->co[id] = NULL;	// reset the slot, and coroutine_status()=COROUTINE_DEAD
	--S->nco;
	S->running = -1;	// reset
}

// stack从高地址向低地址生长, 即从stack bottom向stack top存储数据
void 
coroutine_resume(struct schedule * S, int id) {
	// 当前没有正在运行的协程，否则等待当前协程逻辑单元执行完或yield挂起才能resume下一个。
	assert(S->running == -1);
	assert(id >=0 && id < S->cap);
	struct coroutine *C = S->co[id];
	if (C == NULL)
		return;
	int status = C->status;
	switch(status) {
	case COROUTINE_READY:		// READY->RUNNING
		getcontext(&C->ctx);                  // initialize ucp befor makecontext()
		C->ctx.uc_stack.ss_sp = S->stack;     // stack top起始位置(SP)
		C->ctx.uc_stack.ss_size = STACK_SIZE; // 用于计算stack bottom(数据从stack bottom开始存放)
		C->ctx.uc_link = &S->main;            // 协程执行完切回的context
		S->running = id;                      // 调度器记录当前准备调度的协程id
		C->status = COROUTINE_RUNNING;        // 将准备调度的协程状态置为"待运行状态"
		uintptr_t ptr = (uintptr_t)S;         // 需要考虑跨平台指针大小不同的问题
		// 开始执行mainfunc回调, 执行完继续fall through, 即执行下一个协程
		makecontext(&C->ctx, (void (*)(void)) mainfunc, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));
		swapcontext(&S->main, &C->ctx);       // saves the current thread context in S->main and activate C->ctx
		printf("COROUTINE_READY: coroutine id[%d] return\n", S->running);
		break;
	case COROUTINE_SUSPEND:		// SUSPEND->RUNNING
		// 首次resume运行(READY->RUNNING)时，makecontext 之前设置每个 coroutine::ctx.uc_stack.ss_sp 都指向 S->stack
		// 唤醒挂起的协程，真正执行前，将虚拟私有栈 C->stack（堆上空间） 拷贝到公用交换栈 S->stack（栈上空间） 中
		memcpy(S->stack + STACK_SIZE - C->size, C->stack, C->size);
		S->running = id;
		C->status = COROUTINE_RUNNING;
		swapcontext(&S->main, &C->ctx);
		// <debug>
		sleep(2);
		printf("COROUTINE_SUSPEND: coroutine id[%d] return\n", S->running);
		// </debug>
		break;
	default:
		assert(0);
	}
}

static void
_save_stack(struct coroutine *C, char *top) {
	// 在stack上创建一个局部变量, 标识当前栈顶(SP)的位置(低地址)
	char dummy = 0;                        // stack::top
	printf("_save_stack: &C[%p] top[%p] &dummy[%p] top - &dummy[%d] STACK_SIZE[%d]\n", &C, top, &dummy, top - &dummy, STACK_SIZE);
	// 检查stack是否有溢出
	assert(top - &dummy <= STACK_SIZE);
	// 按需保存当前协程的stack (begin)
	// 判断协程栈的空间是否足够, 若不够则重新分配
	if (C->cap < top - &dummy) {
		free(C->stack);
		C->cap = top-&dummy;              // ebp-esp
		C->stack = malloc(C->cap);        // 堆上分配的一块空间虚拟成栈来使用
	}
	C->size = top - &dummy;                // ebp-esp
	memcpy(C->stack, &dummy, C->size);     // 将当前协程运行时的栈帧活动区拷贝到私有“栈”
	// 按需保存当前协程的stack (end)
}

// 挂起当前正在执行的协程：RUNNING->SUSPEND
void
coroutine_yield(struct schedule * S) {
	int id = S->running;
	printf("coroutine_yield: coroutine id[%d] into\n", S->running);
	assert(id >= 0);	// 当前有正在运行的协程
	struct coroutine * C = S->co[id];
	// 与栈顶S->stack的位置进行比较
	printf("coroutine_yield: &C[%p] S->stack[%p]\n", &C, S->stack);
	assert((char *)&C > S->stack);
	// 按需动态保存协程的private stack
	_save_stack(C,S->stack + STACK_SIZE);
	C->status = COROUTINE_SUSPEND;
	S->running = -1;	// reset
	swapcontext(&C->ctx , &S->main);
}

int 
coroutine_status(struct schedule * S, int id) {
	assert(id>=0 && id < S->cap);
	if (S->co[id] == NULL) {
		return COROUTINE_DEAD;
	}
	return S->co[id]->status;
}

int 
coroutine_running(struct schedule * S) {
	return S->running;
}

