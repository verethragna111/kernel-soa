/*
 *  kernel/sched.c
 *
 *  Minikernel (versión 2.0)
 *
 *  Fernando Pérez Costoya
 *
 */

/* Fichero que contiene la funcionalidad de la planificación */

#include "HAL.h"
#include "sched.h"
#include "common.h"
#include "list.h"
#include "bitmask.h"
#include "process.h" 
#include "clock.h"

/* Variable que representa la cola de procesos listos */
list ready_list[NR_PRIO];

unsigned int ready_queues_mask = 0;

//Variable global de lista de procesos bloqueados
//list sleep_queue;


// Inicializa las estructuras de datos del planificador
void init_sched_module(void) {
    for (int i = 0; i < MAX_PRIO; i++) {
        list_init(&ready_list[i]);
    }
    //list_init(&sleep_queue);
    ready_queues_mask = 0;
    register_irq_handler(SW_INT, soft_interrupt_handler);
    return;
}

/* Espera a que se produzca una interrupcion */
static void wait_for_int(void){
    int level;

    printk("-> NO HAY LISTOS. ESPERA INTERRUPCIÓN\n");

    /* Baja al mínimo el nivel de interrupción mientras espera */
    level=set_int_priority_level(LEVEL_1);
    halt();
    set_int_priority_level(level);
}

// añade un proceso a la cola de listos
void add_ready_queue(PCB *p) {
    int level = set_int_priority_level(LEVEL_3);
    int prio=p->priority;
    p->state=READY;
    insert_last(&ready_list[prio], p);
    set_bit(&ready_queues_mask, prio);

    // Comprobar si se necesita expulsar el proceso actual
    if (current == NULL || p->priority > current->priority) {
        if (current != NULL) {
            // Si el proceso actual no es nulo, activar interrupción software
            activate_soft_int();
            
        } else {
            // Si no hay proceso actual, activar el próximo
            pick_and_activate_next_task(1);
        }
    }
    set_int_priority_level(level);
}


// elimina el proceso actual de la cola de listos
void remove_ready_queue(void) {

    int level = set_int_priority_level(LEVEL_3);
    int prio = current->priority;
    remove_elem(&ready_list[prio], current);
    if (list_is_empty(&ready_list[prio])) {
        clear_bit(&ready_queues_mask, prio);
    }
    set_int_priority_level(level);
}

/* Función de planificacion  */
/* complejidad O(n) */
PCB * scheduler(void) {
    while (ready_queues_mask == 0) wait_for_int();  // nada que hacer

    int highest_prio = find_last_bit_set(ready_queues_mask);
    iterator it;
    iterator_init(&ready_list[highest_prio], &it);
    PCB *p = NULL;
    p = iterator_next(&it);
    return p;
}

void soft_interrupt_handler(void) {
    pick_and_activate_next_task(1); // Salva el contexto del proceso actual
}
