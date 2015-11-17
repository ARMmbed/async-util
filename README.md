# Async Utilities
async-util is a library inspired by [async.js](https://github.com/caolan/async). At present, a single utility is provided: async::Series.

# Series
Series is intended to execute a list of tasks in order, sequentially. Series interprets each function passed to it as the initiator of the corresponding task, rather than the complete task. Because Series interprets the functions this way, it waits for a callback before executing the next function in the list.

Series encapsulates the following programming idiom:

```Python
def Series(functions, handler, fin, cb):
 e = None
 for func in functions:
     try:
         func()
     except:
         e = handler():
         if e:
             break
         else:
             continue
 if fin:
     fin(cb, e)
 else:
     cb(e)
```

## Detailed Description
Series supplies a callback to each function that is called. This is a multi-purpose callback that can be used
to indicate completion or failure of a function. The callback need not be executed within the context of the called
function. It can be deferred to a later time. Series tracks the execution state of the functions supplied to it and
suspends execution until the supplied callback has been called.

Series supports a number of features to make it more useful in typical applications, such as asynchronous operation, error handling, an exit handler, a series complete-callback, a termination callback, and stack-depth control.

### The Series Callback
The Series callback is responsible for processing the success or error state of each function that is called. If an error is passed to the callback, the error handler will be invoked. If no error is passed to the series callback, the next function in the series is executed. The Series callback is also passed to the error handler. If an error is passed to the Series callback while in an error handler, the Series is terminated. If there are no more functions to execute, the series exit handler is called.

**NOTE:** Any function that calls the Series callback should return immediately afterwards. The timing for execution of code following the Series callback is undefined.

### Asynchronous Operation
Series supports deferring execution of a callback until a later time. It accomplishes this by only executing code on entry to the supplied callback. Regardless of the current state of Series, exiting a called function without having called the supplied callback is safe. This is to allow scheduling execution of the supplied callback at a later time. This applies to normal functions, the error handler, and the exit handler.

### Error Handling
Series handles errors by passing an error variable to the Series callback. The Series callback requires that the error variable support two conditions:

* It must be castable to a boolean.
* The default constructor must initialize the error variable such that it is castable to false.

When an error is encountered in a normal function, the function should initialize an error variable such that it is castable to true, then call the Series callback and return immediately.

The series callback will call the error handler with a function pointer to the series callback and the error variable. The error handler can then call the series callback (either immediately or at a later time, as described in Asynchronous Operation) with an error variable.  If the error variable is castable to true, the Series has failed to complete, and the exit handler will be called. If the error variable is castable to false, the error was recoverable and the Series will continue execution at the next function.

### Exit Handler
The Exit Handler is optional and provides the Series with a way to call one more piece of user-supplied code before calling the termination callback. While this has very similar semantics to the termination callback, it allows user-supplied code to be executed--whereas the termination callback may be supplied by another utility, for example a Series of Series objects. The exit handler also allows the execution of the termination callback to be deferred to a later time.

### Termination Callback
The termination callback is supplied by the caller when ```Series::go(cb)``` is called. The termination callback is the last function to be called in the Series. It is used to indicate the completion of the Series to the code that invoked the Series.

### Stack Depth Control
To prevent the stack depth from expanding with every successive function call, all non-error calls to the Series callback will be deferred to the least-nested Series callback. Series tracks entries to and exits from the Series callback, so that it can determine the level of nesting at any point.

## Examples

### Print Three Words
This example calls three print functions to print three words, in order. No finally case is provided, no error handler is provided, and no termination callback is provided.

```C++
#include <stdio.h>
#include "async-util/Series.hpp"
typedef async::Series<int> iSeries;
iSeries ser;

void p1(iSeries::DoneCB cb)
{
    printf("one");
    cb(0);
}
void p2(iSeries::DoneCB cb)
{
    printf("two");
    cb(0);
}
void p3(iSeries::DoneCB cb)
{
    printf("three\n");
    cb(0);
}

void app_start(int argc, char * argv[]) {
    (void) argc;
    (void) argv;
    ser.call(iSeries::Action(p1))
       .then(iSeries::Action(p2))
       .then(iSeries::Action(p3))
       .go();
}
```

### Print Three Words Three Times
This example builds on the previous example, but nests one series inside another. This demonstrates the ```callable``` API, and shows the use of the completion callback.

```C++
#include <stdio.h>
#include "async-util/Series.hpp"
typedef async::Series<int> iSeries;
iSeries ser;
iSeries oser;

void p1(iSeries::DoneCB cb)
{
    printf("one");
    cb(0);
}
void p2(iSeries::DoneCB cb)
{
    printf("two");
    cb(0);
}
void p3(iSeries::DoneCB cb)
{
    printf("three\n");
    cb(0);
}

void app_start(int argc, char * argv[]) {
    (void) argc;
    (void) argv;
    ser.call(iSeries::Action(p1))
       .then(iSeries::Action(p2))
       .then(iSeries::Action(p3));
    oser.call(ser.callable())
        .then(ser.callable())
        .then(ser.callable())
        .go();
}
```
