//
// driver for qemu's virtio disk device.
// uses qemu's mmio interface to virtio.
// qemu presents a "legacy" virtio interface.
//
// qemu ... -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
//

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"
#include "virtio.h"

// the address of virtio mmio register r.
#define R(r) ((volatile uint32 *)(VIRTIO0 + (r)))

struct {
  struct buf *b;
  char status;
  struct spinlock vdisk_lock;
  char free;
}  __attribute__ ((aligned (PGSIZE)))  disk;
  

void
virtio_disk_init(void)
{
  initlock(&disk.vdisk_lock, "virtio_disk");
  disk.free = 1;
}

// find a free descriptor, mark it non-free, return its index.
static int
alloc_desc()
{
  if (disk.free) {
    disk.free = 0;
    return 0;
  }
  return -1;
}

// mark a descriptor as free.
static void
free_desc(int i)
{
  disk.free = 1;
  wakeup(&disk.free);
}

#define EDISK_ADDR  ((volatile unsigned int*)0xf8000000)
#define EDISK_WEN   ((volatile unsigned int*)0xf8000008)
#define EDISK_DATA  ((volatile unsigned int*)0xf8000010)

void
virtio_disk_rw(struct buf *b, int write)
{
  // printf("virtio_disk_rw\n");
  acquire(&disk.vdisk_lock);
  // printf("blockno: %p\n", b->blockno);

  // とりあえず1つdescを奪う
  int idx;
  while (1) {
    if ((idx = alloc_desc()) >= 0) {
      break;
    }
    // 奪えなかったらfreeで寝る
    sleep(&disk.free, &disk.vdisk_lock);
  }

  b->disk = 1;

  unsigned int offset = b->blockno * BSIZE;

  // printf("BSIZE : %p\n", BSIZE);
  // printf("offset: %p\n", offset);

  *EDISK_WEN = write ? 1 : 0;
  for (int i = 0; i < BSIZE / 4; i++) {
    int addr = offset + i * 4;
    *EDISK_ADDR = addr;
    if (write) {
      *EDISK_DATA = ((unsigned int*)b->data)[i];
      // printf("disk_rw(%d): %p <= %p... %d\n", write, addr, ((unsigned int*)b->data)[i], i);
    } else {
      ((unsigned int*)b->data)[i] = *EDISK_DATA;
      // printf("disk_rw(%d): %p => %p... %d\n", write, addr, ((unsigned int*)b->data)[i], i);
    }
  }

  b->disk = 0;
  free_desc(idx);

  release(&disk.vdisk_lock);
}

void
virtio_disk_intr()
{
  panic("virtio_disk_intr");
}
