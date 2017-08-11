
## macOS 编译工具链  LLVM 版本

```Shell
faner@THOMASFAN-MB0:~|⇒  gcc -v
Configured with: --prefix=/Applications/Xcode.app/Contents/Developer/usr --with-gxx-include-dir=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk/usr/include/c++/4.2.1
Apple LLVM version 8.1.0 (clang-802.0.38)
Target: x86_64-apple-darwin16.7.0
Thread model: posix
InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin

faner@THOMASFAN-MB0:~|⇒  g++ -v
Configured with: --prefix=/Applications/Xcode.app/Contents/Developer/usr --with-gxx-include-dir=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk/usr/include/c++/4.2.1
Apple LLVM version 8.1.0 (clang-802.0.38)
Target: x86_64-apple-darwin16.7.0
Thread model: posix
InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin
```

## macOS 编译运行 coroutine
### 编译
```Shell
⇒  
faner@THOMASFAN-MB0:~/Projects/git/framework/coroutine|master⚡ 
⇒  mkdir out
faner@THOMASFAN-MB0:~/Projects/git/framework/coroutine|master⚡ 
⇒  gcc coroutine.c main.c -o out/coroutine 
coroutine.c:141:3: warning: implicitly declaring library function 'getcontext' with type
      'int (ucontext_t *)' (aka 'int (struct __darwin_ucontext *)') [-Wimplicit-function-declaration]
                getcontext(&C->ctx);                  // initialize ucp befor makecontext()
                ^
coroutine.c:141:3: note: include the header <setjmp.h> or explicitly provide a declaration for
      'getcontext'
coroutine.c:149:3: warning: implicit declaration of function 'makecontext' is invalid in C99
      [-Wimplicit-function-declaration]
                makecontext(&C->ctx, (void (*)(void)) mainfunc, 2, (uint32_t)ptr, (uint32_t)(p...
                ^
coroutine.c:150:3: warning: implicit declaration of function 'swapcontext' is invalid in C99
      [-Wimplicit-function-declaration]
                swapcontext(&S->main, &C->ctx);       // saves the current thread context in S...
                ^
coroutine.c:161:3: warning: implicit declaration of function 'sleep' is invalid in C99
      [-Wimplicit-function-declaration]
                sleep(2);
                ^
coroutine.c:174:102: warning: format specifies type 'int' but the argument has type 'long' [-Wformat]
  ...top[%p] &dummy[%p] top - &dummy[%d] STACK_SIZE[%d]\n", &C, top, &dummy, top - &dummy, STACK_SI...
                                     ~~                                      ^~~~~~~~~~~~
                                     %ld
5 warnings generated.
```

### 运行

```Shell
faner@THOMASFAN-MB0:~/Projects/git/framework/coroutine|master⚡ 
⇒  out/coroutine 
main start
mainfunc: coroutine id[0]
coroutine 0 : 0
coroutine_yield: coroutine id[0] into
coroutine_yield: &C[0x10498df48] S->stack[0x10488e000]
_save_stack: &C[0x10498df18] top[0x10498e000] &dummy[0x10498df0f] top - &dummy[241] STACK_SIZE[1048576]
COROUTINE_READY: coroutine id[-1] return
mainfunc: coroutine id[1]
coroutine 1 : 77127520
coroutine_yield: coroutine id[1] into
coroutine_yield: &C[0x10498df48] S->stack[0x10488e000]
_save_stack: &C[0x10498df18] top[0x10498e000] &dummy[0x10498df0f] top - &dummy[241] STACK_SIZE[1048576]
COROUTINE_READY: coroutine id[-1] return
Assertion failed: (0), function coroutine_resume, file coroutine.c, line 166.
[1]    6036 abort      out/coroutine
faner@THOMASFAN-MB0:~/Projects/git/framework/coroutine|master⚡ 
⇒  
```

### 分析
