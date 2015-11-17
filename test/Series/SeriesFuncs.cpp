#include "minar/minar.h"
#include "mbed-drivers/mbed.h"
#include "SeriesFuncs.hpp"

bool SeriesFuncsTrace = false;
struct testCase_s Actual;

unsigned seq;
uint8_t funcTypes[] = {0,0,1,1,2,2,3,3};
void (*TestFunctions[])(iSeries::DoneCB cb) = {
    ics0, ics1, ice0, ice1, dcs0, dcs1, dce0, dce1
};
const size_t nTestFunctions = sizeof(TestFunctions)/sizeof(void (*)(iSeries::DoneCB cb));

bool callsException[4] = {
    false,true,false,true
};

// Direct call, success
void ics0(iSeries::DoneCB cb) {
    if(SeriesFuncsTrace) printf("%s, seq %d\r\n", __PRETTY_FUNCTION__, seq);
    Actual.funcSeq[0] = seq++;
    cb(0);
}
// Direct call, success
void ics1(iSeries::DoneCB cb) {
    if(SeriesFuncsTrace) printf("%s, seq %d\r\n", __PRETTY_FUNCTION__, seq);
    Actual.funcSeq[1] = seq++;
    cb(0);
}
// Direct call, error
void ice0(iSeries::DoneCB cb) {
    if(SeriesFuncsTrace) printf("%s, seq %d\r\n", __PRETTY_FUNCTION__, seq);
    Actual.funcSeq[2] = seq++;
    cb(1);
}
// Direct call, error
void ice1(iSeries::DoneCB cb) {
    if(SeriesFuncsTrace) printf("%s, seq %d\r\n", __PRETTY_FUNCTION__, seq);
    Actual.funcSeq[3] = seq++;
    cb(1);
}
// Scheduled call, success
void dcs0(iSeries::DoneCB cb) {
    if(SeriesFuncsTrace) printf("%s, seq %d\r\n", __PRETTY_FUNCTION__, seq);
    Actual.funcSeq[4] = seq++;
    minar::Scheduler::postCallback(cb.bind(0));
}
// Scheduled call, success
void dcs1(iSeries::DoneCB cb) {
    if(SeriesFuncsTrace) printf("%s, seq %d\r\n", __PRETTY_FUNCTION__, seq);
    Actual.funcSeq[5] = seq++;
    minar::Scheduler::postCallback(cb.bind(0));
}
// Scheduled call, error
void dce0(iSeries::DoneCB cb) {
    if(SeriesFuncsTrace) printf("%s, seq %d\r\n", __PRETTY_FUNCTION__, seq);
    Actual.funcSeq[6] = seq++;
    minar::Scheduler::postCallback(cb.bind(1));
}
// Scheduled call, error
void dce1(iSeries::DoneCB cb) {
    if(SeriesFuncsTrace) printf("%s, seq %d\r\n", __PRETTY_FUNCTION__, seq);
    Actual.funcSeq[7] = seq++;
    minar::Scheduler::postCallback(cb.bind(1));
}

unsigned errMax;

void ei(iSeries::DoneCB cb, int i)
{
    (void) i;
    if(SeriesFuncsTrace) printf("%s, seq %d\r\n", __PRETTY_FUNCTION__, seq);
    int rc = (errMax <= Actual.errors)?1:0;
    Actual.errSeq[Actual.errors] = seq++;
    if(!rc) {
        Actual.errors++;
    }
    cb(rc);
}
void ed(iSeries::DoneCB cb, int i)
{
    (void) i;
    if(SeriesFuncsTrace) printf("%s, seq %d\r\n", __PRETTY_FUNCTION__, seq);
    int rc = (errMax <= Actual.errors)?1:0;
    Actual.errSeq[Actual.errors] = seq++;
    if(!rc) {
        Actual.errors++;
    }
    minar::Scheduler::postCallback(cb.bind(rc));
}
