//
// simple filtration
//
// http://robocraft.ru
//

#ifndef _FILTRATION_H_
#define _FILTRATION_H_

#include <stdio.h>

#ifdef __cplusplus

template <typename T, size_t TSize>
class TSimpleFiltration
{
public:
    TSimpleFiltration() {
        zero();
    }
    ~TSimpleFiltration() {
    }

    inline void add(T val) {
        if(counter < TSize) {
            data[counter] = val;
            ++counter;
        }
        else {
            counter = 0;
            is_valid = true;
        }
        if(is_valid) {
            calc();
        }
    }

    inline T calc() {
        T summ = 0;
        for(size_t i=0; i<TSize; i++) {
            summ += data[i];
        }
        prev_value = value;
        value = summ/TSize;

        return value;
    }

    inline void zero() {
        memset(data, 0, sizeof(data));
        value = 0;
        prev_value = 0;

        counter = 0;
        is_valid = false;
    }

    T data[TSize];
    T value;
    T prev_value;

    size_t counter;
    bool is_valid;
};

#endif //#ifdef __cplusplus

#endif //#ifndef _FILTRATION_H_
