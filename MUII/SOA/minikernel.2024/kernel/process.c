/*
 *  kernel/process.c
 *
 *  Minikernel (versión 2.0)
 *
 *  Fernando Pérez Costoya
 *
 */

/* Fichero que contiene la funcionalidad de la gestión de procesos */

#include "HAL.h"
#include "process.h"
#include "sched.h"
#include "syscall.h"
#include "common.h"
#include "list.h"
#include "clock.h"
#include "bitmask.h"


// Variable global que identifica el proceso actual
PCB * current = NULL;

/* Variable global que representa la tabla de procesos */
static PCB proc_table[MAX_NR_PROC];

list dormidos; 

/* Función que inicia la tabla de procesos */
static void init_process_table(void){
    for (int i=0; i<MAX_NR_PROC; i++) proc_table[i].state = FINISHED;
}

/* Función que busca una entrada libre en la tabla de procesos.
   Asigna PIDs en orden creciente reciclando cuando hay desbordamiento */
static int search_free_PCB(void){
    static int nproc = MAX_NR_PROC, next = 0;
    for (int i=next; nproc--; i = (i + 1) % MAX_NR_PROC)
        if (proc_table[i].state == FINISHED)  {
            next = (i + 1) % MAX_NR_PROC;
            return i;
        }
    return -1;
}

// el proceso actual no puede seguir ejecutando;
// hay que salvar su contexto y activar el proceso elegido por planificador;
// si parámetro es 0, no salva el contexto.
void pick_and_activate_next_task(int save_ctx) {
    PCB *prev = current;
    current=scheduler();
    current->state = RUNNING;
    if (prev != current) {
        if (prev) printk("-> CAMBIO DE CONTEXTO DE %d A %d\n",
                prev->pid, current->pid);
        context *ctx = (prev && save_ctx) ? &(prev->context) : NULL;
        context_switch(ctx, &(current->context));
    }
}

/* Implementación de la llamada al sistema create_process */
int do_create_process(char *prog, int prio){
    void * image, *initial_pc;
    int error=0;
    int nr_proc;
    PCB *p_new;

    if (check_syscall_arg_string_read(prog, MAX_EXEC_NAMELENGTH) == -1)
        return -1;

    if ((prio > MAX_PRIO) || (prio < MIN_PRIO)) return -1;


    image=create_image(prog, &initial_pc);
    if (image) {
        nr_proc=search_free_PCB();
        if (nr_proc==-1) return -1;    /* no hay entrada libre */
        /* A rellenar el PCB ... */
        p_new=&(proc_table[nr_proc]);
        p_new->mem=image;
        p_new->stack=create_stack(STACK_SIZE);
        set_initial_context(p_new->mem, p_new->stack, STACK_SIZE,
            initial_pc, &(p_new->context));
        p_new->pid=nr_proc;
        p_new->priority=prio;

        printk("-> NUEVO PROCESO %d\n", p_new->pid);

        /* lo inserta en la cola de listos */
        add_ready_queue(p_new);
        error= 0;
    }
    else
        error= -1; /* fallo al crear imagen */
    return error;
}
/* Implementación de la llamada al sistema exit_process */
int do_exit_process(void){
    printk("-> TERMINA PROCESO %d\n", current->pid);
    release_image(current->mem);
    current->state=FINISHED;
    remove_ready_queue();
    release_stack(current->stack);
    pick_and_activate_next_task(0); // no salva estado del previo
    return 0; /* no debería llegar aquí ya que el proceso terminó */
}
void init_process_module(void) {
    init_process_table();
}

int do_get_pid(void){

    /* Codigo de prueba de excepcion aritmetica modo sistema 
    
    int x = 1, y = 0;
    x /= y; // excepción aritmética
    return x;
    */

   /* Codigo de prueba de excecion de memoria modo sistema 
   int *pi = 0;
    *pi = 5; // excepción de memoria*/

    return current->pid;
}

int do_get_priority(void){
    return current->priority;
}

int do_proc_sleep(unsigned int secs){

    int level = set_int_priority_level(LEVEL_3);

    current->state=BLOCKED;
    current->wake_up_time = secs*100;
    remove_ready_queue();
    add_sleep_queue(current);

    set_int_priority_level(level);
    pick_and_activate_next_task(1);
    return 0; 
}

void counter_down(void){
    iterator it; 
    iterator_init(&dormidos, &it);
    while (iterator_has_next(&it))
    {
        PCB * p = (PCB *)iterator_next(&it);
        int contador = p->wake_up_time -1; 
        p->wake_up_time = contador; 

        if(p->wake_up_time == 0){
            remove_sleep_queue(p);
        }
    }
}

void add_sleep_queue(PCB *p){
    insert_last(&dormidos,p);
}

void remove_sleep_queue(PCB *p){
    remove_elem(&dormidos, p);
    add_ready_queue(p);
}
