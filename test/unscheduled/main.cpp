
#include "mbed-drivers/mbed.h"
#include "async-util/Series.hpp"
#include "minar/minar.h"

using namespace mbed::util;

typedef async::v0::Series<int> iSeries;

void f0(iSeries * s)
{
    printf("%s\r\n", __PRETTY_FUNCTION__);
    s->CallNext(0);
}
void f1(iSeries * s)
{
    printf("%s\r\n", __PRETTY_FUNCTION__);
    FunctionPointer1<void, int> fp(s, &iSeries::CallNext);
    minar::Scheduler::postCallback(fp.bind(1));
    //s->CallNext(0);
}
void f2(iSeries * s)
{
    printf("%s\r\n", __PRETTY_FUNCTION__);
    s->CallNext(0);
}
void fin(iSeries * s) {
    (void) s;
    printf("Done!\r\n");
}
void except(iSeries * s, int) {
    (void) s;
    printf("Sadface :(\r\n");
    s->CallNext(0);
}


extern "C" void app_start(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    static Serial pc(USBTX,USBRX);
    pc.baud(115200);
    static iSeries ser;
    ser.call(FunctionPointer1<void, iSeries *>(f0))
       .then(FunctionPointer1<void, iSeries *>(f1))
       .then(FunctionPointer1<void, iSeries *>(f2))
       .except(FunctionPointer2<void, iSeries *, int>(except))
       .finally(FunctionPointer1<void, iSeries *>(fin));
    minar::Scheduler::postCallback(&ser, &iSeries::go);
}
