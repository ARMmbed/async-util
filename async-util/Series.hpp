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
#ifndef __CORE_UTIL_SERIES_H__
#define __CORE_UTIL_SERIES_H__

/**
 * \file
 * \brief Asynchronous Programming Series Operations
 * The series class provides a container for initiating operations in sequence. An error handler is provided
 * using the except() member function. A final operation to call if all functions are successful is provided
 * using the finally() member function.
 */

#include "core-util/FunctionPointerBase.h"
#include "core-util/Array.h"
#include "ualloc/ualloc.h"
namespace mbed {
namespace util {
namespace async {
namespace v0 {

#define ARRAY_GROWBY 4


/**
 * @brief A generic sequential operation container
 * Series supports calling functions in order, but does not require that each operation be wholly contained in the
 * function which is called. This allows asynchronous operations to be executed in sequence, but only requires tracking
 * the Series object itself.
 *
 * To create a series object, use one of the two constructors. The default constructor can be tuned to the expected
 * number of arguments.
 *
 * To indicate that a function has completed, call CallNext with an argument that evaluates to false.
 *
 * To indicate an error, call CallNext with an argument that evaluates to true.
 *
 * To set an error handler, call except().
 *
 * To set a function to call when all functions have completed successfully, call finally()
 *
 * To call the first function in the series, call go().
 *
 * Error codes are handled via CallNext(). The template of Series allows configuring the error handling in a type-safe
 * way. <typename E> must be castable to bool. If E evaluates to false, no error is handled. If E evaluates to true,
 * the handler set in except() is called.
 */
template<typename E>
class Series {
public:
    typedef mbed::util::FunctionPointer1<void, E> DoneCB_t;
    typedef mbed::util::FunctionPointer1<void, DoneCB_t> Action_t;
    typedef mbed::util::FunctionPointer2<void, DoneCB_t, E> ExceptionHandler_t;
    typedef mbed::util::Array<Action_t> SeriesArray_t;

    /**
     * Create a new, empty series.
     * The default size is 2 actions. For best performance, this size should be set to the expected number of arguments.
     * @param[in] initial_size The number of elements with which to initialize the array
     * @param[in] traits The allocator traits to use for the action array (default: none)
     * @param[in] alignment The alignment to use for the action array (default: 4)
     */
    Series(const unsigned initial_size = 2, UAllocTraits_t traits = UAllocTraits_t(),
        unsigned alignment = MBED_UTIL_POOL_ALLOC_DEFAULT_ALIGN)
        : _onError(ExceptionHandler_t(NULL)),
        _finally(ExceptionHandler_t(NULL)),
        _doneCB(DoneCB_t(NULL)),
        _index(0), _inCB(false), _callNext(false), _inErrCB(false), _errNeedsResolution(false), _done(false)
    {
        _callList.init(initial_size, ARRAY_GROWBY, traits, alignment);
    }
    /**
     * Create a new series, using an existing series.
     * The existing series is copied into the new series, but traits and alignment
     * are overridden with those provided.
     * @param[in] series An existing series
     * @param[in] traits The allocator traits to use for the action array (default: none)
     * @param[in] alignment The alignment to use for the action array (default: 4)
     */
    Series(const Series &series, UAllocTraits_t traits = UAllocTraits_t(),
        unsigned alignment = MBED_UTIL_POOL_ALLOC_DEFAULT_ALIGN)
        :_onError(ExceptionHandler_t(NULL)),
        _finally(ExceptionHandler_t(NULL)),
        _doneCB(DoneCB_t(NULL)),
        _index(0), _inCB(false), _callNext(false), _inErrCB(false), _errNeedsResolution(false), _done(false)
    {
        const unsigned init_elements = series._callList.get_num_elements();
        _callList.init(init_elements, ARRAY_GROWBY, traits, alignment);
        call(series);
    }
    /**
     * Add an existing Series to the call list
     * Copies the elements of the existing Series into a new one
     * @param[in] series The existing series to call
     * @return A reference to the series object
     */
    Series & call(const Series & series)
    {
        const unsigned init_elements = series._callList.get_num_elements();
        for (unsigned i = 0; i < init_elements; i++) {
            _callList.push_back(series._callList[i]);
        }
        return *this;
    }
    /**
     * Add a function to the call list
     * Appends the function to the end of the call list
     * @param[in] series The existing series to call
     * @return A reference to the series object
     */
    Series & call(const Action_t & action)
    {
        _callList.push_back(action);
        return *this;
    }
    /**
     * Synonym for call()
     * Appends the function to the end of the call list
     * @param[in] series The existing series to call
     * @return A reference to the series object
     */
    inline Series & then(const Action_t & action) {
        return call(action);
    }
    /**
     * Set the error handler
     * The function passed to except is called when the argument to CallNext is castable to true
     * @param[in] handler The function to call when E indicates an error
     * @return A reference to the series object
     */
    Series & except(const ExceptionHandler_t & handler)
    {
        _onError = handler;
        return *this;
    }

    /**
     * Set the function to call when done
     * The function passed to except is called when all functions in the series have
     * executed successfully.
     * @param[in] handler The function to call when the series is done
     * @return A reference to the series object
     */
    Series & finally(const ExceptionHandler_t & handler)
    {
        _finally = handler;
        return *this;
    }
    /**
     * Start the series
     * The done callback is passed to the finally callback.
     * @param[in] done a handler to call when the series completes
     */
    void go(DoneCB_t done = DoneCB_t(NULL))
    {
        _index = 0;
        _inCB = false;
        _callNext = false;
        _doneCB = done;
        _inErrCB = false;
        _errNeedsResolution = false;
        _done = false;
        CallNext(E());
        //_callList[_index](DoneCB_t(this, &Series::CallNext));
    }

    Action_t callable(DoneCB_t done)
    {
        return Action_t(this, &Series::go);
    }


    /**
     * Call the next function in the series.
     * Because this handler can be called under many circumstances, this functions is complex
     * Possible state:
     * +---+--------+-------+------+---------+--------------------------+
     * | e | in Err | in CB | done | onError | Action                   |
     * +---+--------+-------+------+---------+--------------------------+
     * | 1 |    1   |   X   |   X  |    X    | Exit                     |
     * | 1 |    0   |   X   |   X  |    0    | Exit                     |
     * | 1 |    0   |   X   |   X  |    1    | Call onError with e      |
     * | 0 |    1   |   X   |   X  |    X    | Set callNext and return  |
     * | 0 |    0   |   1   |   X  |    X    | Set callNext and return  |
     * | 0 |    0   |   0   |   1  |    X    | Exit                     |
     * | 0 |    0   |   0   |   0  |    X    | Call next function       |
     * +---+--------+-------+------+---------+--------------------------+
     *
     * If CallNext is called recursively, it should always defer processing to less nested calls
     * CallNext should ignore defered processing
     *
     * @param[in] e An error type. When e is castable to true, an error is indicated
     */
    void CallNext(E e)
    {
        if (_done) {
            return;
        }
        if (e) {
            if (_errNeedsResolution) {
                exit(e);
                return;
            } else if (_onError) {
                _errNeedsResolution = true;
                _inErrCB = true;
                _onError(DoneCB_t(this, &Series::CallNext), e);
                _inErrCB = false;
                if (_errNeedsResolution) {
                    return;
                }
            } else {
                exit(e);
                return;
            }
        } else if (_errNeedsResolution) {
            _errNeedsResolution = false;
        }
        if (_inErrCB || _inCB) {
            _callNext = true;
            _errNeedsResolution = false;
            return;
        }
        do {
            _callNext = false;
            _inCB = true;
            if (_index >= _callList.get_num_elements()) {
                exit(E());
                _callNext = false;
            } else {
                _callList[_index](DoneCB_t(this, &Series::CallNext));
                _index++;
            }
            _inCB = false;
        } while (_callNext && !_done && !_errNeedsResolution);
    }
protected:
    void exit(E e)
    {
        _done = true;
        if (_finally && _doneCB) {
            _finally(_doneCB, e);
        } else if (_finally && !_doneCB) {
            _finally(DoneCB_t(&Series::nullcb), e);
        } else if (_doneCB) {
            _doneCB(e);
        }
    }

protected:
    static void nullcb(E e) {(void)e;}
    SeriesArray_t _callList;
    ExceptionHandler_t _onError;
    ExceptionHandler_t _finally;
    DoneCB_t _doneCB;
    unsigned _index;
    bool _inCB;
    bool _callNext;
    bool _inErrCB;
    bool _errNeedsResolution;
    bool _done;
};

}
}
}
}

 #endif // __CORE_UTIL_SERIES_H__
