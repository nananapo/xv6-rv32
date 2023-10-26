#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

void
uartinit(void)
{
}

// write one output character to the UART.
void
uartputc(int c)
{
  volatile unsigned int *addr = (unsigned int*)0xff000000;
  *addr = c;
}

// read one input character from the UART.
// return -1 if none is waiting.
int
uartgetc(void)
{
  volatile unsigned int *en     = (unsigned int*)0xff000010;
  volatile unsigned int *rdata  = (unsigned int*)0xff000018;
  if(*en & 0x01){
    // input data is ready.
    return *rdata;
  } else {
    return -1;
  }
}

// trap.c calls here when the uart interrupts.
void
uartintr(void)
{
  while(1){
    int c = uartgetc();
    if(c == -1)
      break;
    consoleintr(c);
  }
}
