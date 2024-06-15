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
static char t_buffer[TERM_BUF_SIZE];
static fifo t_fifo;
static list blocked_queue;


/* Funciones relacionadas con la gestión del terminal */

// implementación de la llamada al sistema

void init_terminal_module(void) {
    init_keyboard_controller();
    fifo_init(&t_fifo, sizeof(char), TERM_BUF_SIZE, t_buffer);
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
    int level = set_int_priority_level(LEVEL_2);
    printf("INT. TECLADO\n");
    char c ;
    c = read_port(KEYBOARD_PORT); // leer el carácter desde el hardware
    print_terminal(&c, 1); 
    printk("\n");

    if (!fifo_is_full(&t_fifo)) {
        fifo_in(&t_fifo, &c);
        
        // Desbloquear el primer proceso en la cola de bloqueo si lo hay
        if (!list_is_empty(&blocked_queue)) {
            PCB * p = queue_priority(&blocked_queue);
            remove_elem(&blocked_queue,p);
            add_ready_queue(p);
        }
    } else {
        printk("Buffer lleno, carácter descartado\n");
    }
    set_int_priority_level(level);
}

int do_get_char(void) {
    char c;

    while (fifo_is_empty(&t_fifo)) {
        int level = set_int_priority_level(LEVEL_2);
        current->state = BLOCKED;
        insert_last(&blocked_queue,current);
        remove_ready_queue();
        pick_and_activate_next_task(1);
        set_int_priority_level(level);
    }

    int level = set_int_priority_level(LEVEL_2);
    fifo_out(&t_fifo,&c);
    printk("Caracter: %c",c);
    set_int_priority_level(level);

    return c;
}