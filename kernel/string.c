// 主要是涉及到内存拷贝、比较、设置以及字符串的比较、拷贝、复制的操作
#include "types.h"

// 这个函数的作用是把 dst 指向的内存区域的前 n 个字节设置为字符 c
// memory set
void*
memset(void *dst, int c, uint n)
{
  char *cdst = (char *) dst;
  int i;
  for(i = 0; i < n; i++){
    cdst[i] = c;
  }
  return dst;
}

// 这个函数的作用是比较两块内存区域的前 n 个字节的内容是否相同。
// 如果不同就返回不同字节两值之差，相同就返回0
// memory compare
int
memcmp(const void *v1, const void *v2, uint n)
{
  const uchar *s1, *s2;

  s1 = v1;
  s2 = v2;
  while(n-- > 0){
    if(*s1 != *s2)
      return *s1 - *s2;
    s1++, s2++;
  }

  return 0;
}
// 第一个参数——需要拷贝到用户空间对应的物理地址
// 第二个参数——源操作数据的起始地址
// 第三个参数——拷贝字节数
// 在两个地址进行操作时都会进行内核虚拟地址转换，不过是直接映射
void*
memmove(void *dst, const void *src, uint n)
{
  const char *s;
  char *d;

  if(n == 0)
    return dst;
  
  s = src;
  d = dst;
  // 这种情况考虑的就是当源数据和目标数据有重叠，并且源数据的起始地址在目标数据的起始地址之前的情况
  // 这种情况下进行从头部开始的数据复制会导致部分源数据未复制前被修改，最终导致复制错误
  // 如果这种情况发生，那么 s < d && s + n > d 就会返回 true。
  // 在这种情况下，我们不能从头到尾进行复制，而应该从尾到头进行复制。这就是 *--d = *--s 的作用。
  if(s < d && s + n > d){
    s += n;
    d += n;
    // 从尾开始循环拷贝
    while(n-- > 0)
      *--d = *--s;
  } else
  // 从头开始循环拷贝
    while(n-- > 0)
      *d++ = *s++;

  return dst;
}

// memcpy exists to placate GCC.  Use memmove.
void*
memcpy(void *dst, const void *src, uint n)
{
  return memmove(dst, src, n);
}

// 相同返回0，不同返回不同字节值的差
int
strncmp(const char *p, const char *q, uint n)
{
  // 这里有一个细节——就是为什么我们不要保证*q!=0,
  // 因为当*q==0的时候，这两个条件中(*p && *p == *q)一定会有一个条件为false
  while(n > 0 && *p && *p == *q)
    n--, p++, q++;
  if(n == 0)
    return 0;
  return (uchar)*p - (uchar)*q;
}

char*
strncpy(char *s, const char *t, int n)
{
  char *os;

  os = s;
  while(n-- > 0 && (*s++ = *t++) != 0)
    ;
  while(n-- > 0)
    *s++ = 0;
  return os;
}

// Like strncpy but guaranteed to NUL-terminate.
// 就是说保证了最后会加入终止符"\0"
char*
safestrcpy(char *s, const char *t, int n)
{
  char *os;
  // 用于保存原始的s地址
  os = s;
  if(n <= 0)
    return os;
  // 这里有个细节就是但第一个条件不成立的时候，while不会执行第二个条件，直接跳出循环
  // 所以在进行到最后会将终止符加入到复制结果最后面
  while(--n > 0 && (*s++ = *t++) != 0)
    ;
  *s = 0;
  return os;
}

// strlen函数设计的初衷是为了计算一个以\0（即ASCII的0）结尾的C字符串的长度
// 只能计算字符串长度
// 如果我们将这个函数直接用于一个通用的数组，那么当数组中的元素值为0时，
// 函数就会提前返回长度，这并不准确。因为在非字符串的字节数组中，0x00（\0）并不代表字符串的结束，它只是一个正常的数据。
int
strlen(const char *s)
{
  int n;
  // 要注意一点:在 C 语言（以及大多数其他编程语言）中，字符常量通常被编译器转换为它们的 ASCII 码值进行计算。
  // 例如，如果你写下 'A'，编译器会把它看作 65
  // 而对于for语句而言，第二个是判断语句，当为true的时候会进入循环体，false的时候会跳出循环体
  // 而在C语言中"\0"对应就是空字符也就对应于ASCII的0,我们知道当整数为0的时候,是false
  // 所以下面这个for语句就是循环判断是否到了字符数组的末尾"\0"，来记录字符数组的字节数
  for(n = 0; s[n]; n++)
    ;
  return n;
}

