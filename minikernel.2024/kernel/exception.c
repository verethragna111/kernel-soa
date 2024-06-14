/*
 *  kernel/exception.c
 *
 *  Minikernel (versión 2.0)
 *
 *  Fernando Pérez Costoya
 *
 */

#include "HAL.h"
#include "exception.h"
#include "common.h"
#include "process.h"
#include "syscall.h"

/* Funciones relacionadas con el tratamiento de excepciones */

void init_exception_module(void) {
    register_irq_handler(ARITM_EXC,arithmetic_exception_handler);

    register_irq_handler(MEM_EXC,memory_exception_handler);
    return; 
}

void arithmetic_exception_handler(void){
    if(comes_from_usermode()){
        do_exit_process();
    }
    else{
        panic("excepción aritmetica en modo sistema");
    }
    return;
}

void memory_exception_handler(void){
    if (comes_from_usermode()) {
        do_exit_process(); 
    } 
    else if(check_arg==0) {
        
        panic("Excepción de memoria en modo sistema");
    } 
    else{
        check_arg=0;
        do_exit_process(); 
    }
    return;
}
