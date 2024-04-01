// 多层hash表
// 通过dev号以及块号进行hash寻找到对应的buf指针
#include "hashtable.h"
#include "types.h"
#include "buf.h"
#include <stdbool.h>

struct hashEntry
{
    struct buf *buffer;
};

struct hashLevel
{
    struct hashEntry *hashtable;
    int length;
};

struct hashCache
{
    struct hashLevel hashleveltable[HASH_LEVELS];
};

static uint8 levelTable[HASH_LEVELS] = {LEVEL0, LEVEL1, LEVEL2, LEVEL3, LEVEL4};
// 初始化多层hash表
void hinit()
{
    // struct hashEntry hashtable[5] = {};

    // 定义五层hash表
    for (int i = 0; i < HASH_LEVELS; i++)
    {
    }
    struct hashEntry hashtable0[LEVEL0] = {};
    struct hashEntry hashtable1[LEVEL1] = {};
    struct hashEntry hashtable2[LEVEL2] = {};
    struct hashEntry hashtable3[LEVEL3] = {};
    struct hashEntry hashtable4[LEVEL4] = {};
    struct hashLevel hashLevel0 = {hashtable0, LEVEL0};
    struct hashLevel hashLevel1 = {hashtable1, LEVEL1};
    struct hashLevel hashLevel2 = {hashtable2, LEVEL2};
    struct hashLevel hashLevel3 = {hashtable3, LEVEL3};
    struct hashLevel hashLevel4 = {hashtable4, LEVEL4};
    struct hashCache *hcache;
    hcache->hashleveltable[LEVEL0] = hashLevel0;
    hcache->hashleveltable[LEVEL1] = hashLevel1;
    hcache->hashleveltable[LEVEL2] = hashLevel2;
    hcache->hashleveltable[LEVEL3] = hashLevel3;
    hcache->hashleveltable[LEVEL4] = hashLevel4;
}

// 定义hash函数
// 哈希函数要根据层数来进行计算
uint8 hash(uint dev, uint blockno, struct hashEntry *hashtable, int level)
{
    uint8 index = (dev * 30 + blockno) % levelTable[level];
    return index;
}

// 定义插入
// 返回-2表示数据无法写入，返回-1表示数据已经存在，返回0表示正常插入
int hash_insert(uint dev, uint blockno, struct hashCache *hcache, struct buf *buffer)
{
    struct buf *buffer0 = hash_search(dev, blockno, hcache);
    if (buffer0)
    {
        return -1;
    }
    for (int i = 0; i < HASH_LEVELS; i++)
    {
        buffer0 = hash_level_search(dev, blockno, hcache, i);
        if (!buffer0)
        {
            // 把数据写入
            buffer0 = buffer;
            return 0;
        }
    }
    return -2;
}

// 定义行查找
struct buf *hash_level_search(uint dev, uint blockno, struct hashCache *hcache, int level)
{
    uint8 index = hash(dev, blockno, hcache, level);
    struct buf *buffer = (hcache->hashleveltable[level].hashtable)[index].buffer;
    return buffer;
}

// 定义查找
struct buf *hash_search(uint dev, uint blockno, struct hashCache *hcache)
{
    struct buf *buffer;
    for (int i = 0; i < HASH_LEVELS; i++)
    {
        buffer = hash_level_search(dev, blockno, hcache, i);
        if (buffer && buffer->dev == dev && buffer->blockno == blockno)
        {
            return buffer;
        }
    }
    return 0;
}

// 定义删除
// 返回-1表示hashcache中不存在该数据,返回0表示数据正确删除
// 前移功能主要是提高平均查找效率
int hash_remove(uint dev, uint blockno, struct hashCache *hcache)
{
    struct buf *buffer = hash_search(dev, blockno, hcache);
    struct buf *buffer1;
    // 这个标志表明是否开启循环前移(就是表示删除的数据后面层次还有相关数据)
    bool forward = false;
    if (buffer)
    {
        return -1;
    }
    for (int i = 0; i < HASH_LEVELS; i++)
    {
        buffer = hash_level_search(dev, blockno, hcache, i);
        // 表示到达最后一次，buffer1应该为空指针
        if (i >= 4)
        {
            buffer1 = 0;
        }
        else
        {
            buffer1 = hash_level_search(dev, blockno, hcache, i + 1);
        }
        // 判断是否是该buf
        if (buffer && buffer->dev == dev && buffer->blockno == blockno)
        {   
            // 当后面层次无数据(就是没有因为hash碰撞导致向后层存储的buf指针)或者当当前层次是最后一层的时候
            if (!buffer1)
            {   
                // 把该buf删除
                buffer = buffer1;
                return 0;
            }
            // 有后面层次有数据的时候
            forward = true;
        }
        // 循环前移哈希碰撞的数据
        if (forward)
        {   
            // 后面层次数据前移
            buffer = buffer1;
            // 如果后面层次无数据或者当前到达最后一层
            if (!buffer1)
            {
                return 0;
            }
        }
    }

    return 0;
}
