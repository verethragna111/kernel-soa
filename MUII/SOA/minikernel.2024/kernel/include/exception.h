/*
 *  kernel/include/exception.h
 *
 *  Minikernel (versión 2.0)
 *
 *  Fernando Pérez Costoya
 *
 */

#ifndef _EXCEPTION_H
#define _EXCEPTION_H

void init_exception_module(void); // función de iniciación del módulo

void arithmetic_exception_handler(void);

void memory_exception_handler(void);

#endif /* _EXCEPTION_H */

