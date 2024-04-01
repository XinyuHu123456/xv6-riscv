// Sleeping locks
// 这一部分被我进行了修稿
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"

void
initsleeplock(struct sleeplock *lk, char *name)
{
  initlock(&lk->lk, "sleep lock");
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
  lk->inPtr = 0;
  lk->outPtr = 0;
  lk->sleeping[NMaxSleepProc] = "";
  // 初始化头指针head
  lk->head = allocSleepProcNode(0);
}
/*
// sleepinglocksleep
void
slSleep(void *chan)
{
  struct proc *p = myproc();
  
  // Must acquire p->lock in order to
  // change p->state and then call sched.
  // Once we hold p->lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup locks p->lock),
  // so it's okay to release lk.

  acquire(&p->lock);  //DOC: sleeplock1
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;
  sched();
  // Tidy up.
  p->chan = 0;
  // Reacquire original lock.
  release(&p->lock);
}
*/
void
acquiresleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  // 当获取不到锁的时候，就会sleep，当被唤醒以后，就是继续执行
  // 但是会发生一种情况，就是一个新的进程也来获取这个锁，正好跟这个刚唤醒的这个进程同时进行到while语句
  // 都调过了,都进行lk->locked = 1,这时候就会发生冲突了
  // 怎么解决? 所以这里还是要一个锁机制，这里其实也能够明白为什么sleep，需要传入一个锁的原因了
  // 那就又回到了原来的实现方案了，经过这一系列思考你会发现原来的操作虽然效率低，但是保证了安全
  // 在保证安全的同时也会降低效率
  while (lk->locked) {
    // slSleep(lk);
    
    // 这里判断一下如果当前pid是链表第一个进程pid
    // 就说明是新唤醒的进程但是没有抢到锁
    if(){
      // 这时候继续在头结点睡眠

    }else{
      lk->outPtr = lk->outPtr == 0? 0:(lk->outPtr+1)% NMaxSleepProc;
      // Sleep(lk,&lk->lk);
      // 这里如果是第一个挂载的进程跟新来的进程去抢，没抢到就会被挂到链表最后，这个是不合理的
      // 换一种结构不使用数组而是使用链表
      Sleep(&lk->sleeping[lk->outPtr],&lk->lk);
    }
  }
  // 这里判断一下如果当前pid是链表第一个进程pid
    // 就说明是新唤醒的进程但是抢到了锁
    if(){
      // 这时候把头结点弹出
    }
  lk->locked = 1;
  lk->pid = myproc()->pid;
  release(&lk->lk);
}
/* 问题描述
  对于多个进程同时访问文件,如果有很多个以上的进程同时需要对文件进行操作
  按照之前的时候会有1个进程获得sleeplock的spinlock，其他多个进程都会自旋等待，这样其实十分浪费CPU资源
  所以我们通过取消sleeplock的spinlock，但是只用一个sleeplock也是不行的，如果多个进程同时访问,则会有多个进程睡眠
  但是按照之前这个wakeup逻辑，如果仅仅使用一个chan(多个进程都是这个chan导致sleeping的)，这样wakeup的时候会导致
  所有的在这个锁上睡眠进程都会被唤醒，但是又只有一个进程得到这个锁，也同样造成了很多CPU资源浪费(多进程切换)
*/
// —————————解决办法———————————
// 通过简单实现一个先睡先唤醒的队列来支持多个进程睡眠

// 经过上面的思考发现还是需要spinlock，但是新出现了一个问题，就是在同时被唤醒的时候，多个进程会在sleep里面同时自旋获取
// lk->lk，这又有了一个新的解决方案，再次引入先睡先唤醒的队列来支持多个进程睡眠，而同一时刻只有一个进程唤醒
// 要重新实现一下Sleep
void
releasesleep(struct sleeplock *lk)
{
  acquire(&lk->lk);
  lk->locked = 0;
  lk->pid = 0;
  wakeup(&lk->sleeping[lk->inPtr]);
  // lk->inPtr = (lk->inPtr+1)% NMaxSleepProc;
  release(&lk->lk);
}

// 这个是用来判断同一个进程多次acquire同一个锁的情况
int
holdingsleep(struct sleeplock *lk)
{
  int r;
  
  acquire(&lk->lk);
  r = lk->locked && (lk->pid == myproc()->pid);
  release(&lk->lk);
  return r;
}

struct sleepProcNode * allocSleepProcNode(int pid){
  struct sleepProcNode node = {pid,0,0};
  return &node; 
}

// 节点插入
// 使用尾插法
void insertSleepList(struct sleepProcNode * node,struct sleeplock *lk){
  struct sleepProcNode * head = lk->head;
  node->prev = head->prev;
  node->next = head;
  head->prev->next = node;
  head->prev = node;
}

// 弹出头结点
void removeSleepList(struct sleeplock *lk){
  struct sleepProcNode * head = lk->head;
  head->next->next->prev = head;
  head->next = head->next->next;
}