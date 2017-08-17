- **LIBRARY**

     Standard C Library (libc, -lc)

- **SYNOPSIS**

```C
#include <ucontext.h>
```

- **NAME**

    getcontext, setcontext -- get and set user thread context
    makecontext, swapcontext -- modify and exchange user thread contexts

- **Description**

In a System V-like environment, one has the two types *`mcontext_t`* and *`ucontext_t`* defined in <ucontext.h> and the four functions `getcontext()`, `setcontext()`, `makecontext`(3) and `swapcontext`(3) that allow *user-level* context switching between **multiple threads** of control within a ***process***.

The *`mcontext_t`* type is machine-dependent and opaque. The *`ucontext_t`* type is a structure that has at least the following fields:

```C
typedef struct ucontext {
    struct ucontext *uc_link;
    sigset_t         uc_sigmask;
    stack_t          uc_stack;
    mcontext_t       uc_mcontext;
    ...
} ucontext_t;
```

*`uc_mcontext`* is the machine-specific representation of the saved context, that includes the calling thread's machine registers.

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

---
The **makecontext**() function modifies the context pointed to by *ucp* (which was obtained from a call to **getcontext**(3)). Before invoking **makecontext**(), the caller must allocate a new stack for this context and assign its address to *`ucp->uc_stack`*, and define a successor context and assign its address to *`ucp->uc_link`*.

When this context is later activated (using **setcontext**(3) or **swapcontext**()) the function *func* is called, and passed the series of integer (int) arguments that follow argc; the caller must specify the number of these arguments in argc. When this function returns, the successor context is ***activated***. If the successor context pointer is NULL, the thread exits.

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

## macOS
```C
// usr/include/ucontext.h

/*
 * These routines are DEPRECATED and should not be used.
 */
#ifndef _UCONTEXT_H_
#define _UCONTEXT_H_

#include <sys/cdefs.h>

#ifdef _XOPEN_SOURCE
#include <sys/ucontext.h>
#include <Availability.h>

#else /* !_XOPEN_SOURCE */
#error The deprecated ucontext routines require _XOPEN_SOURCE to be defined
#endif /* _XOPEN_SOURCE */

#endif /* _UCONTEXT_H_ */

```

## Conforming To
SUSv2, POSIX.1-2001. POSIX.1-2008 removes the specification of `getcontext()`, citing portability issues, and recommending that applications be rewritten to use POSIX threads **instead**.
