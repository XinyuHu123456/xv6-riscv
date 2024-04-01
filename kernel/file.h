struct file {
  enum { FD_NONE, FD_PIPE, FD_INODE, FD_DEVICE } type;
  int ref; // reference count
  char readable;
  char writable;
  // 这里其实可以做一个小小的优化，就是使用一个联合体来减少8个字节内存的使用
  // union{
      // struct pipe *pipe;
      // struct inode *ip;
  // }dataPtr;
  struct pipe *pipe; // FD_PIPE
  struct inode *ip;  // FD_INODE and FD_DEVICE
  // 下面也是同样的道理，可以使用union来减少内存的使用
  // union{
  //   uint off;    
  //   short major;      
  // }metaData;
  
  // 表示偏移量
  uint off;          // FD_INODE
  short major;       // FD_DEVICE
};

/* 内核中如何区分设备号
设备号是在驱动module中分配并注册的，而/dev目录下的设备文件是根据这个设备号创建的，
因此，当访问/dev目录下的设备文件时，驱动module就知道，自己该出场服务了(当然是由内核通知)。
在Linux内核看来，主设备号标识设备对应的驱动程序，
告诉Linux内核使用哪一个驱动程序为该设备(也就是/dev下的设备文件)服务；
而次设备号则用来标识具体且唯一的某个设备。在内核中，用dev_t类型
(其实就是一个32位的无符号整数)的变量来保存设备的主次设备号，其中高12位表示主设备号，低20位表示次设备号。

设备获得主次设备号有两种方式：一种是手动给定一个32位数，并将它与设备联系起来(即用某个函数注册)；
另一种是调用系统函数给设备动态分配一个主次设备号。
*/
// 与主次设备号相关的3个宏
// 主设备号标识设备对应的驱动程序，告诉Linux内核使用哪一个驱动程序为该设备(也就是/dev下的设备文件)服务；
// 而次设备号则用来标识具体且唯一的某个设备,(比如磁盘不同分区，可以通过次设备区分)
#define major(dev)  ((dev) >> 16 & 0xFFFF)
#define minor(dev)  ((dev) & 0xFFFF)
#define	mkdev(m,n)  ((uint)((m)<<16| (n)))

// in-memory copy of an inode
struct inode {
  uint dev;           // Device number
  uint inum;          // Inode number
  int ref;            // Reference count
  struct sleeplock lock; // protects everything below here
  int valid;          // inode has been read from disk?

  short type;         // copy of disk inode
  short major;
  short minor;
  short nlink;
  // 存储的是文件大小
  uint size;
  // 这里要注意一点就是:这里实际分配了NDIRECT大小的直接块和一个大小的indirect的间接块
  // 这也就是为什么这里NDIRECT+1，多出来的一个是间接块号
  // uint其实表示的是4个字节的inode号

  uint addrs[NDIRECT+1];
};

// map major device number to device functions.
// device switch设备选择数组
struct devsw {
  int (*read)(int, uint64, int);
  int (*write)(int, uint64, int);
};

extern struct devsw devsw[];

#define CONSOLE 1
#define TUN 200
