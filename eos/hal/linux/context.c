#include <core/eos.h>
#include <core/eos_internal.h>

/*
 * How EOS saves a context of a task in the stack memory.
 */
typedef struct _os_context {
	/* low address */
    int32u_t edi_reg;
    int32u_t esi_reg;
    int32u_t ebp_reg;
    int32u_t esp_reg;
    int32u_t ebx_reg;
    int32u_t edx_reg;
    int32u_t ecx_reg;
    int32u_t eax_reg;
    int32u_t eflags_reg;
    int32u_t eip_reg;
	/* high address */	
} _os_context_t;

/*
 * print_context - For debugging. Print all the contents of the given context
 *  in the order they resident in the memory.
 */
void print_context(addr_t context) {
	if (context == NULL) return;
	_os_context_t *ctx = (_os_context_t *)context;

	PRINT("eax    = 0x%x\n", ctx->eax_reg);
	PRINT("ebx    = 0x%x\n", ctx->ebx_reg);
	PRINT("ecx    = 0x%x\n", ctx->ecx_reg);
	PRINT("edx    = 0x%x\n", ctx->edx_reg);
	PRINT("edi    = 0x%x\n", ctx->edi_reg);
	PRINT("esi    = 0x%x\n", ctx->esi_reg);
	PRINT("ebp    = 0x%x\n", ctx->ebp_reg);
	PRINT("esp    = 0x%x\n", ctx->esp_reg);
	PRINT("eflags = 0x%x\n", ctx->eflags_reg);
	PRINT("eip    = 0x%x\n", ctx->eip_reg);
}

/*
 * _os_create_context - Create new empty context based on the entry of main
 *  routine of the new task. Returns new stack pointer.
 */
addr_t _os_create_context(addr_t stack_base, size_t stack_size, void (*entry)(void *), void *arg) {
    addr_t stack_bottom = (addr_t)(((int32u_t)stack_base) + stack_size);
    void **arg_param = ((void **)stack_bottom) - 1;
    addr_t *ret_addr = ((addr_t *)arg_param) - 1;
    addr_t top = (addr_t)(((_os_context_t *)ret_addr) - 1);
    _os_context_t *ctx = (_os_context_t *)top;

    /* Initialize the new stack */
    *arg_param = arg;
    *ret_addr = NULL;
    ctx->eax_reg = (int32u_t)NULL;
    ctx->ebx_reg = (int32u_t)NULL;
    ctx->ecx_reg = (int32u_t)NULL;
    ctx->edx_reg = (int32u_t)NULL;
    ctx->edi_reg = (int32u_t)NULL;
    ctx->esi_reg = (int32u_t)NULL;
    ctx->ebp_reg = (int32u_t)NULL;
    ctx->esp_reg = (int32u_t)NULL;
    ctx->eflags_reg = (int32u_t)NULL;
    ctx->eip_reg = (int32u_t)entry;

    return top;
}

/*
 * _os_restore_context - Restore context based on the stack pointer. First it
 *  begins with the given stack, restores all the registers those have been
 *  saved. The routine finalizes with returning to the last saved state of
 *  the indicated execution stream.
 */
void _os_restore_context(addr_t sp) {
    __asm__ __volatile__("\
        mov %0, %%esp;\
        popa;\
        pop _eflags;\
        ret;"
        ::"m"(sp));
}

/*
 * _os_save_context - Saves all the (integral) registers on the stack. Returns
 *  the stack pointer, and the current thread of control is stopped. When the
 *  context is restored, the execution begins right after the first return
 *  statement, where the second return is held. The routine then returns 0.
 */
addr_t _os_save_context() {
    __asm__ __volatile__("\
        push $resume_eip;\
        push _eflags;\
        mov $0, %%eax;\
        pusha;\
        mov %%esp, %%eax;\
        push 0x4(%%ebp);\
        push 0x0(%%ebp);\
        mov %%esp, %%ebp;\
    resume_eip:"
        :);
}
