
#include "mbed-drivers/mbed.h"
#include "async-util/Series.hpp"
#include "minar/minar.h"
#include "mbed-drivers/test_env.h"

using namespace mbed::util;

typedef async::v0::Series<int> iSeries;

size_t seq = 1;
#define NTESTS 5
size_t CallCheck[NTESTS] = {0};

void f0(iSeries::DoneCB_t cb)
{
    CallCheck[0] = seq++;
    printf("f0\r\n");
    cb(0);
}
void f1(iSeries::DoneCB_t cb)
{
    CallCheck[1] = seq++;
    printf("f1\r\n");
    minar::Scheduler::postCallback(cb.bind(1));
    //s->CallNext(0);
}
void f2(iSeries::DoneCB_t cb)
{
    CallCheck[3] = seq++;
    printf("f2\r\n");
    cb(0);
}
void fin(iSeries::DoneCB_t cb)
{
    CallCheck[4] = seq++;
    printf("fin\r\n");
    cb(0);
}
void except(iSeries::DoneCB_t cb, int i)
{
    printf("Exception %i\r\n", i);
    CallCheck[2] = seq++;
    cb(0);
}
void check(int i) {
    (void) i;
    bool passed = true;
    for(size_t p = 0; p < NTESTS; p++) {
        if (CallCheck[p] != p+1) {
            passed = false;
        }
    }
    MBED_HOSTTEST_RESULT(passed);
}


extern "C" void app_start(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    static Serial pc(USBTX,USBRX);
    pc.baud(115200);
    MBED_HOSTTEST_TIMEOUT(5);
    MBED_HOSTTEST_SELECT(default);
    MBED_HOSTTEST_DESCRIPTION(Series Test);
    MBED_HOSTTEST_START("Series");
    static iSeries ser;


    ser.call(iSeries::Action_t(f0))
       .then(iSeries::Action_t(f1))
       .then(iSeries::Action_t(f2))
       .except(iSeries::ExceptionHandler_t(except))
       .finally(iSeries::Action_t(fin));
    minar::Scheduler::postCallback(iSeries::Action_t(&ser, &iSeries::go).bind(check));
}
