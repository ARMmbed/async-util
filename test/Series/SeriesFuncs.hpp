/* mbed Microcontroller Library
 * Copyright (c) 2015 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __SERIESFUNCS_HPP__
#define __SERIESFUNCS_HPP__

#include "core-util/FunctionPointer.h"
#include "async-util/Series.hpp"
using namespace mbed::util;

typedef async::v0::Series<int> iSeries;

struct testCase_s {
    void (*functions[8])(mbed::util::FunctionPointer1<void, int>);
    void (*exception)(mbed::util::FunctionPointer1<void, int>, int);
    unsigned funcSeq[8];
    unsigned errSeq[4];
    unsigned errors;
    unsigned finSeq;
    bool fin;
    int cbExpect;
};
extern struct testCase_s Actual;
extern const size_t nTests;

extern uint8_t funcTypes[];
extern void (*TestFunctions[])(iSeries::DoneCB cb);
extern const size_t nTestFunctions;
extern bool callsException[4];

// Direct call, success
void ics0(iSeries::DoneCB cb);
// Direct call, error
void ice0(iSeries::DoneCB cb);
// Scheduled call, success
void dcs0(iSeries::DoneCB cb);
// Scheduled call, error
void dce0(iSeries::DoneCB cb);
// Direct call, success
void ics1(iSeries::DoneCB cb);
// Direct call, error
void ice1(iSeries::DoneCB cb);
// Scheduled call, success
void dcs1(iSeries::DoneCB cb);
// Scheduled call, error
void dce1(iSeries::DoneCB cb);

void ei(iSeries::DoneCB cb, int i);
void ed(iSeries::DoneCB cb, int i);

extern unsigned errMax;
extern unsigned seq;


#endif // __SERIESFUNCS_HPP__
