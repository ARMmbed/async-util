
#include "mbed-drivers/mbed.h"
#include "async-util/Series.hpp"
#include "minar/minar.h"
#include "mbed-drivers/test_env.h"
#include "SeriesFuncs.hpp"
#include <algorithm>


void fin(iSeries::DoneCB cb, int i)
{
    Actual.finSeq = seq++;
    cb(i);
}

unsigned testIdx;
bool success = true;
const unsigned MaximumErrors = 4;
iSeries *ser;

static void test();

// Initialize with the initial test
struct testCase_s CurrentTest = {
    .functions = {ics0, ics1, ice0, ice1, dcs0, dcs1, dce0, dce1},
    .exception = ei,
    .funcSeq = {1, 2, 3, 0, 0, 0, 0, 0},
    .errSeq  = {4, 0, 0, 0},
    .errors = 0,
    .finSeq = 0,
    .fin = false,
    .cbExpect = 1
};

void generate(struct testCase_s *testCase) {
    unsigned gSeq = 1;
    unsigned eCnt = 0;
    bool seen[4] = {false, false, false, false};
    memset(&testCase->funcSeq, 0, sizeof(testCase->funcSeq));
    memset(&testCase->errSeq, 0, sizeof(testCase->errSeq));
    for (int i = 0; i < 8; i++) {
        uint8_t funcType = funcTypes[i];
        unsigned funcIdx = funcType*2;
        if (!seen[funcType]) {
            seen[funcType] = true;
        } else {
            funcIdx++;
        }
        testCase->functions[i] = TestFunctions[funcIdx];
        testCase->funcSeq[funcIdx] = gSeq++;
        if (callsException[funcType]) {
            testCase->errSeq[eCnt++] = gSeq++;
        }
        if (eCnt > testCase->errors){
            break;
        }
    }
    testCase->finSeq = (testCase->fin)?gSeq++:0;
    testCase->cbExpect = (eCnt > testCase->errors?1:0);
}

bool next(struct testCase_s *testCase) {
    if (testCase->errors < MaximumErrors) {
        testCase->errors++;
    } else if (testCase->exception == ei) {
        testCase->exception = ed;
        testCase->errors = 0;
    } else if (!testCase->fin) {
        testCase->exception = ei;
        testCase->errors = 0;
        testCase->fin = true;
    } else if (std::next_permutation(funcTypes,funcTypes+nTestFunctions)) {
        testCase->exception = ei;
        testCase->errors = 0;
        testCase->fin = false;
    } else {
        return false;
    }
    generate(testCase);
    return true;
}


void check(int i) {
    Actual.cbExpect = i;
    if (Actual.finSeq) {
        Actual.fin = true;
    }
    delete ser;
    if (0 != memcmp(&Actual, &CurrentTest, sizeof(Actual))) {
        MBED_HOSTTEST_RESULT(0);
    }
    if (next(&CurrentTest)) {
        testIdx++;
        test();
    } else {
        testIdx++;
        printf("Tested %u possible calling combinations of Series\r\n",testIdx);
        MBED_HOSTTEST_RESULT(1);
    }
}

void defer_check(int i) {
    minar::Scheduler::postCallback(FunctionPointer1<void,int>(check).bind(i));
}


void test() {
    // Generate test case
    ser = new iSeries;
    memset(&Actual, 0, sizeof(Actual));
    memcpy(Actual.functions, CurrentTest.functions, sizeof(Actual.functions));
    Actual.exception = CurrentTest.exception;

    ser->call(iSeries::Action(CurrentTest.functions[0]));
    for (int i = 1; i < 8; i++) {
        ser->then(iSeries::Action(CurrentTest.functions[i]));
    }
    ser->except(iSeries::ErrorHandler(CurrentTest.exception));
    if (CurrentTest.fin){
        ser->finally(iSeries::ErrorHandler(fin));
    }
    seq = 1;
    errMax = CurrentTest.errors;
    if ((testIdx & 0xFF) == 0){
        printf("Test %u\r\n",testIdx);
    }
    minar::Scheduler::postCallback(iSeries::Action(ser, &iSeries::go).bind(defer_check));
}
extern "C" void app_start(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    static Serial pc(USBTX,USBRX);
    pc.baud(115200);
    MBED_HOSTTEST_TIMEOUT(120);
    MBED_HOSTTEST_SELECT(default);
    MBED_HOSTTEST_DESCRIPTION(Series Test);
    MBED_HOSTTEST_START("Series");
    testIdx = 0;
    test();
}
