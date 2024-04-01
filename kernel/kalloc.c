// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

// 我认为是kernel data的最大地址+1(为什么对于PHYSTOP等它们都是在该区域顶部的地址+1，也就是其他页的起始地址)
extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

// 至于为什么内存分配器会使用freelist而不是使用bitmap，
// 请你看我的相关文章<<对于内存和磁盘进行空间分配方法的讨论与理解>>以及DRAM的物理结构
struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  // 这里告诉我们内核是怎么初始化physical page allocator的
  // 其实就是从内核data段顶部的第一个页开始然后到PHYSTOP地址空间,不断向上遍历每个页进行free再加入到freelist当中
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);
  // 其实freelist存储的就是可以分配页的地址的链表
  r = (struct run*)pa;

  acquire(&kmem.lock);
  // 使用头插法将释放后的页插入freelist
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.

// 这里要注意一点，在C语言中空指针是用0来表示，表示不指向任何地址
// 而在C++里面通常使用的是NULL
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  // 把r赋值为freelist中指向的下一个run结构体的指针
  r = kmem.freelist;
  // 如果指针非空，就表示freelist不为0也就表明有空闲页可以分配
  if(r)
    // 将准备分配的页从frrelist删除
    kmem.freelist = r->next;
  release(&kmem.lock);
  // 这里其实在else的情况下，也就是没有空闲页的情况下，应该是要使用内存置换算法来获取空闲页
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  // 返回可以分配的页地址
  return (void*)r;
}
