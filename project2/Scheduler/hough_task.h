#ifndef HOUGH_TASK_H_
#define HOUGH_TASK_H_

#include "../hough/hough.c"

int hough_task(){
    volatile char dummyVar;
    dummyVar = houghTransform( (uint16_t) &red, (uint16_t) &green, (uint16_t) &blue );
    return 1;
}

#endif