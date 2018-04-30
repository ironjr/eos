/********************************************************
 * Filename: core/task.c
 * 
 * Author: parkjy, RTOSLab. SNU.
 * 
 * Description: task management.
 ********************************************************/
#include <core/eos.h>

#define READY		1
#define RUNNING		2
#define WAITING		3

/*
 * Queue (list) of tasks that are ready to run.
 */
static _os_node_t *_os_ready_queue[LOWEST_PRIORITY + 1];

/*
 * Pointer to TCB of running task
 */
static eos_tcb_t *_os_current_task;

/*
 * eos_create_task - Create task on the given entry point with arguments.
 *  Allocated stack memory and task priority is given.
 */
int32u_t eos_create_task(eos_tcb_t *task, addr_t sblock_start, size_t sblock_size, void (*entry)(void *arg), void *arg, int32u_t priority) {
    addr_t task_sp;
    task_sp = _os_create_context(sblock_start, sblock_size, entry, arg);

    task->state = READY;
    task->priority = priority;
    task->sp = task_sp;

    /* Enqueue the new task to the ready queue */
    task->node = (_os_node_t *)malloc(sizeof(_os_node_t));
    task->node->ptr_data = task;
    task->node->priority = priority;
    _os_add_node_tail(&_os_ready_queue[priority], task->node);
    _os_set_ready(priority);

	PRINT("task: 0x%x, priority: %d\n", (int32u_t)task, priority);

    return 0;
}

int32u_t eos_destroy_task(eos_tcb_t *task) {
}

/*
 * eos_schedule - Scheduler dispatch new task from the ready queue. First it
 *  saves the current context if exists, then it restores another context from
 *  the next appropriate task.
 */
void eos_schedule() {
    eos_tcb_t *current_task = eos_get_current_task();

    if (current_task != NULL) {  
        addr_t prev_task_sp = _os_save_context();

        /* Woke up by the scheduler */
        if (prev_task_sp == NULL) return;

        int32u_t priority = current_task->priority;

        /* Save current context to tcb. */
        current_task->sp = prev_task_sp;

        /* Enqueue current task to the ready queue. */
        current_task->state = READY;
        _os_add_node_tail(&_os_ready_queue[priority], current_task->node);
        _os_set_ready(priority);
    }

    int32u_t work_priority = _os_get_highest_priority();
    _os_node_t *next_task_node = _os_ready_queue[work_priority];

    /* Nothing to schedule. */
    if (next_task_node == NULL) return;

    eos_tcb_t *next_task = next_task_node->ptr_data;

    /* Load next executing task. */
    _os_remove_node(&_os_ready_queue[work_priority], next_task_node);
    if (_os_ready_queue[work_priority] == NULL) _os_unset_ready(work_priority);
    next_task->state = RUNNING;
    _os_current_task = next_task;
    
    /* Restore the next context. */
    _os_restore_context(next_task->sp);
}

eos_tcb_t *eos_get_current_task() {
	return _os_current_task;
}

void eos_change_priority(eos_tcb_t *task, int32u_t priority) {
}

int32u_t eos_get_priority(eos_tcb_t *task) {
}

void eos_set_period(eos_tcb_t *task, int32u_t period){
}

int32u_t eos_get_period(eos_tcb_t *task) {
}

int32u_t eos_suspend_task(eos_tcb_t *task) {
}

int32u_t eos_resume_task(eos_tcb_t *task) {
}

void eos_sleep(int32u_t tick) {
}

void _os_init_task() {
	PRINT("initializing task module.\n");

	/* init current_task */
	_os_current_task = NULL;

	/* init multi-level ready_queue */
	int32u_t i;
	for (i = 0; i < LOWEST_PRIORITY; i++) {
		_os_ready_queue[i] = NULL;
	}
}

void _os_wait(_os_node_t **wait_queue) {
}

void _os_wakeup_single(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_all(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_sleeping_task(void *arg) {
}
