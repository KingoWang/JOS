// evil hello world -- kernel pointer passed to kernel
// kernel should destroy user environment in response

#include <inc/lib.h>
#include <inc/mmu.h>
#include <inc/x86.h>

struct Gatedesc *buf;
struct Gatedesc old;
// Call this function with ring0 privilege

void evil()
{
	// Kernel memory access
	*(char*)0xf010000a = 0;

	// Out put something via outb
	outb(0x3f8, 'I');
	outb(0x3f8, 'N');
	outb(0x3f8, ' ');
	outb(0x3f8, 'R');
	outb(0x3f8, 'I');
	outb(0x3f8, 'N');
	outb(0x3f8, 'G');
	outb(0x3f8, '0');
	outb(0x3f8, '!');
	outb(0x3f8, '!');
	outb(0x3f8, '!');
	outb(0x3f8, '\n');
}

static void
sgdt(struct Pseudodesc* gdtd)
{
	__asm __volatile("sgdt %0" :  "=m" (*gdtd));
}
static void wrapper()
{
    evil();
    buf[4] = old;
    asm volatile("popl %ebp");
    asm volatile("lret");
}
// Invoke a given function pointer with ring0 privilege, then return to ring3
void ring0_call(void (*fun_ptr)(void)) {
    // Here's some hints on how to achieve this.
    // 1. Store the GDT descripter to memory (sgdt instruction)
    // 2. Map GDT in user space (sys_map_kernel_page)
    // 3. Setup a CALLGATE in GDT (SETCALLGATE macro)
    // 4. Enter ring0 (lcall instruction)
    // 5. Call the function pointer
    // 6. Recover GDT entry modified in step 3 (if any)
    // 7. Leave ring0 (lret instruction)

    // Hint : use a wrapper function to call fun_ptr. Feel free
    //        to add any functions or global variables in this 
    //        file if necessary.

    // Lab3 : Your Code Here
    struct Pseudodesc gdtd;
    sgdt(&gdtd);
    cprintf("base %x,limit %x\n",gdtd.pd_base,gdtd.pd_lim);
    void *va = (void*)0x400000;
	//sys_map_kernel_page((void*)gdtd.pd_base,buf);
    if(sys_map_kernel_page((void*)gdtd.pd_base,va)!=0)
    {
        cprintf("error map\n");
    }
    else
    {
        cprintf("map success\n");
    }
    buf = (struct Gatedesc *)(va+PGOFF(gdtd.pd_base));
    old = buf[4];
    SETCALLGATE(buf[4],GD_KT,wrapper,3);
    asm volatile("lcall $0x20,$0");
    cprintf("here\n");
}

void
umain(int argc, char **argv)
{
        // call the evil function in ring0
	ring0_call(&evil);

	// call the evil function in ring3
	evil();
}

