#include<file.h>
// 定义一个缓冲区能接受的最大数据包的个数
#define INPUT_MAXNDATA_SIZE 128
#define OUPUT_MAXNDATA_SIZE 128

struct {
    dataMessage* readq;
    dataMessage* writeq;
} tun;
// 定义一个消息数组

// 定义消息类型
typedef struct message{
    char * data;
    dataMessage * prev;
    dataMessage * next;
}dataMessage;

int
consoleread(int user_dst, uint64 dst, int n)
{}

int
consolewrite(int user_src, uint64 src, int n)
{}
void
tuninit(void)
{
    // 进行设备的初始化
    dataMessage readq[INPUT_MAXNDATA_SIZE];
    dataMessage writeq[OUPUT_MAXNDATA_SIZE];
    tun.readq = readq;
    tun.writeq = writeq;
  


  // connect read and write system calls
  // to consoleread and consolewrite.
    devsw[TUN].read = tunread;
    devsw[TUN].write = tunwrite;
}
