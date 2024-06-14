/*
 *  kernel/clock.c
 *
 *  Minikernel (versión 2.0)
 *
 *  Fernando Pérez Costoya
 *
 */

/* Fichero que contiene la funcionalidad relacionada con el tiempo */

#include "common.h"
#include "HAL.h"
#include "clock.h"
#include "list.h"
#include "process.h"
#include "sched.h"
#include "bitmask.h"


void init_clock_module(void) {
    init_clock_controller(TICK);


    register_irq_handler(CLOCK_INT,clock_exception_handler);

    return;
}

void clock_exception_handler(void){
    counter_down();
    printk("-> INT. RELOJ\n");
    return;
}


