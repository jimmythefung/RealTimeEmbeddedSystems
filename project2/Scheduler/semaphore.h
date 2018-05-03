#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include "common.h"
int semaphore_task(){

    // Turn on yellow LED
    cli();
    event_poll_flag = 0;
    SET_BIT( *_yellow.port, _yellow.pin );
    sei();

    // Busy wait 5ms
    _delay_ms(5);

    // Turn off yellow LED
    cli();
    CLEAR_BIT( *_yellow.port, _yellow.pin );
    sei();

    return 1;
}

#endif