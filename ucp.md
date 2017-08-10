LIBRARY

     Standard C Library (libc, -lc)

SYNOPSIS

```C
#include <ucontext.h>
```

NAME

    getcontext, setcontext -- get and set user thread context
    makecontext, swapcontext -- modify and exchange user thread contexts

## getcontext
```C
int
getcontext(ucontext_t *ucp);
```

The `getcontext()` function saves the current thread's execution context in the structure pointed to by ucp.  
This saved context may then later be restored by calling `setcontext()`.

## makecontext
```C
void
makecontext(ucontext_t *ucp, void (*func)(), int argc, ...);
```

The `makecontext()` function **modifies** the user thread context pointed to by ucp, which must ***have*** previously been *initialized* by a call to `getcontext`(3) and had a stack allocated for it.  
The context is **modified** so that it will **continue** execution by invoking `func()` with the arguments (of type ***int***) provided.
The *`argc`* argument must be equal to the *number* of additional arguments provided to `makecontext()` and also equal to the *number* of arguments to `func()`, or else the behavior is undefined.

The `ucp->uc_link` argument must be initialized ***before*** calling `makecontext()` and determines the action to take when `func()` returns: if equal to NULL, the process exits; otherwise, ***`setcontext(ucp->uc_link)`*** is *implicitly* invoked.

## swapcontext
```C
int
swapcontext(ucontext_t *oucp, const ucontext_t *ucp);
```

The `swapcontext()` function **saves** the current thread context in `*oucp` and makes `*ucp` the currently active context.

## setcontext
```C
int
setcontext(const ucontext_t *ucp);
```

The `setcontext()` function makes a previously saved thread context the current thread context.

## details
1. If <u>ucp</u> was initialized by `getcontext()`, then execution **continues** as if the original `getcontext()` call had just returned (again).

2. If <u>ucp</u> was initialized by `makecontext`(3), execution **continues** with the invocation of the function specified to `makecontext`(3).  
When that function returns, `ucp->uc_link` determines what happens next: 

- if `ucp->uc_link` is NULL, the process exits;   
- otherwise, ***`setcontext(ucp->uc_link)`*** is *implicitly* invoked.  
