/*
 *  kernel/terminal.c
 *
 *  Minikernel (versión 2.0)
 *
 *  Fernando Pérez Costoya
 *
 */


#include "HAL.h"
#include "terminal.h"
#include "syscall.h"
#include "common.h"
#include "process.h"
#include "fifo.h"
#include "list.h"
#include "sched.h"

/* Variables */
static char term_buffer[TERM_BUF_SIZE];
static fifo term_fifo;
static list blocked_queue;


/* Funciones relacionadas con la gestión del terminal */

// implementación de la llamada al sistema

void init_terminal_module(void) {
    init_keyboard_controller();
    fifo_init(&term_fifo, sizeof(char), TERM_BUF_SIZE, term_buffer);
    register_irq_handler(KEYBOARD_INT,keyboard_exception_handler);

    return;
}

int do_print(char * buf, int size) {
    if (size <= 0) return -1;
    if (check_syscall_arg_pointer_read(buf, size) == -1)
        return -1;
    print_terminal(buf, size); // operación del HAL
    return 0;
}

/*
void keyboard_exception_handler(void){
    disable_irq(KEYBOARD_INT);  

    printk("-> INT. TECLADO\n");
    return;
}
*/


void keyboard_exception_handler(void) {
    printk("prueba de que entra \n");
    char c ;
    c = read_port(KEYBOARD_PORT); // leer el carácter desde el hardware
    
    if (!fifo_is_full(&term_fifo)) {
        fifo_in(&term_fifo, &c);
        
        // Desbloquear el primer proceso en la cola de bloqueo si lo hay
        if (!list_is_empty(&blocked_queue)) {
            iterator it; 
            iterator_init(&blocked_queue,&it);
            PCB *process = (PCB *)iterator_next;
            add_ready_queue(process);
        }
    } else {
        printk("Buffer lleno, carácter descartado\n");
    }
}

int do_get_char(void) {
    char c;

    // Inhibir interrupciones del terminal
    int level = set_int_priority_level(LEVEL_3);

    // Verificar si hay datos disponibles en el buffer
    while (fifo_is_empty(&term_fifo)) {
        // No hay datos, bloquear el proceso
        pick_and_activate_next_task(1);

    }

    if(!fifo_is_empty(&term_fifo)){
        fifo_out(&term_fifo,&c);
        printk("Caracter: %c",c);
    }

    // Habilitar interrupciones del terminal
    set_int_priority_level(level);

    return c;
}