#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

volatile static int started = 0;

// start() jumps here in supervisor mode on all CPUs.
void
main()
{
  if(cpuid() == 0){
    consoleinit();
    printfinit();
    printf("\n");
    printf("xv6 kernel is booting\n");
    printf("\n");
    kinit();         // physical page allocator
    // printf("a");
    kvminit();       // create kernel page table
    // printf("b");
    kvminithart();   // turn on paging
    // printf("c");
    procinit();      // process table
    // printf("d");
    trapinit();      // trap vectors
    // printf("e");
    trapinithart();  // install kernel trap vector
    plicinit();      // set up interrupt controller
    plicinithart();  // ask PLIC for device interrupts
    binit();         // buffer cache
    // printf("g");
    iinit();         // inode cache
    // printf("h");
    fileinit();      // file table
    // printf("i");
    virtio_disk_init(); // emulated hard disk
    // printf("j");
    userinit();      // first user process
    // printf("k");
    __sync_synchronize();
    started = 1;
  } else {
    while(started == 0)
      ;
    __sync_synchronize();
    printf("hart %d starting\n", cpuid());
    kvminithart();    // turn on paging
    trapinithart();   // install kernel trap vector
    plicinithart();   // ask PLIC for device interrupts
  }

  scheduler();        
}
