#ifndef SLEEPLOCK_H
#define SLEEPLOCK_H

#define NMaxSleepProc 10
// Long-term locks for processes

// 表明挂载在进程中的节点
// 使用双向链表
struct sleepProcNode {
  int pid;
  struct sleepProcNode* prev;
  struct sleepProcNode* next;
};

struct sleeplock {
  uint locked;       // Is the lock held?
  struct spinlock lk; // spinlock protecting this sleep lock
  // 用来支持多个进程睡眠并高效访问
  char sleeping[NMaxSleepProc];
  // 其实就相当于一个队列，先进先出
  struct sleepProcNode * head;
  uint8 outPtr;
  uint8 inPtr;

  // For debugging:
  char *name;        // Name of lock.
  int pid;           // Process holding lock
};

struct sleepProcNode * allocSleepProcNode(int pid);
void insertSleepList(struct sleepProcNode * node,struct sleeplock *lk);
void removeSleepList(struct sleeplock *lk);

#endif