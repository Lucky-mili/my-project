// 这个头文件只编译一次，无论被#include多少次，都只生效一份代码。
#pragma once

#include <atomic>   // 提供原子操作，用于多线程下无锁、线程安全、高效地计数
#include <cassert>  // 提供断言assert，用于调试时的参数校验，方便定位bug
#include <cstdint>  // 提供标准的固定宽度整数类型（如uint32_t）
#include <iostream>
#include <memory>   // 智能指针，用于管理内存池的资源
#include <mutex>    // 互斥锁，用于实现线程安全的内存池操作

namespace Kama_memoryPool
{
#define MEMORY_POOL_NUM 64 // 总内存池管理器里有 64 个小内存池，也就是分了 64 个“档位”的内存池
#define SLOT_BASE_SIZE 8   // 每个档位的内存块大小，是 SLOT_BASE_SIZE 的倍数
#define MAX_SLOT_SIZE 512  // 内存池最大支持分配 512 字节的内存块，超过这个大小的对象直接用系统的 new/delete 分配


    /*
        Slot 就是内存池里的“内存块节点”，也叫“槽”
        具体内存池的槽的大小没法确定，因为每个内存池的槽的大小不同
        所以这个槽结构体的 sizeof 不是实际的槽大小
    */
    struct Slot
    {
        std::atomic<Slot*> next; // 原子指针
    };

    // 固定大小内存池类
    class MemoryPool
    {
    public:
        MemoryPool(size_t BlockSize = 4096);
        ~MemoryPool();

        void init(size_t);       // 初始化内存池的核心方法，参数为单个槽的大小

        void* allocate();        // 分配槽
        void deallocate(void*);  // 回收槽
    private:
        void allocateNewBlock();                  // 申请新的块并切割为槽
        size_t padPointer(char* p, size_t align); // 内存对齐，把传入的地址 p 向上调整，返回满足 align 对齐要求的地址

        // 使用CAS操作进行无锁入队和出队
        bool pushFreeList(Slot* slot);            // 释放内存时，把槽重新放回 freeList_
        Slot* popFreeList();                      // 分配内存时，从 freeList_ 里取出一个可用的槽
    private:
        int BlockSize_;    // 内存块大小
        int SlotSize_;     // 槽大小
        Slot* firstBlock_; // 指向内存池管理的首个实际大内存块，用来析构时统一释放所有内存，防止泄漏
        Slot* curSlot_;    // 指当前大块中下一个可分配的槽地址
        Slot* lastSlot_;   // 指当前大块中的边界（curSlot_ 超过该位置时需申请新的内存块）
        std::atomic<Slot*> freeList_;         // 指向空闲的槽（被使用过后又被释放的槽）的链表头节点
        //std::mutex       mutexForFreeList_; // 保证 freeList_ 在多线程中操作的原子性
        std::mutex         mutexForBlock_;    // 避免多线程重复开辟内存的锁
    };

    // MemoryPool 的管理和调度器
    class HashBucket
    {
    public:
        static void initMemoryPool();                // 初始化 64 个内存池（只在程序启动时调用一次，把所有内存池都准备好）
        static MemoryPool& getMemoryPool(int index); // 根据索引，拿到对应的 MemoryPool 实例的引用

        // 内存申请接口
        static void* useMemory(size_t size)
        {
            if (size <= 0)
            {
                return nullptr;
            }
            if (size > MAX_SLOT_SIZE)                // 大于512字节的内存，使用 new 申请
            {
                return operator new(size);
            }

            // 相当于 size / 8 向上取整（因为分配内存只能大不能小）
            return getMemoryPool(((size + 7) / SLOT_BASE_SIZE) - 1).allocate();
        }

        // 内存回收接口
        static void freeMemory(void* ptr, size_t size)
        {
            if (!ptr)
            {
                return;
            }
            if (size > MAX_SLOT_SIZE)
            {
                operator delete(ptr);
                return;
            }

            getMemoryPool(((size + 7) / SLOT_BASE_SIZE) - 1).deallocate(ptr);
        }

        // template<>：模板声明
        // typename T：第一个固定类型参数
        // typename... Args：可变参数包（可以接收 0 个、1 个、N 个任意类型）
        // Args&&... args：转发引用，同时处理左值引用和右值引用
        template<typename T, typename... Args>
        friend T* newElement(Args&&... args);     // 在内存池中构造一个对象

        template<typename T>
        friend void deleteElement(T* p);          // 在内存池中销毁一个对象
    };

    // 从内存池中分配一块内存，并在上面构造一个 T 类型的对象
    template<typename T, typename... Args>
    T* newElement(Args&&... args)
    {
        T* p = nullptr;
        // 根据元素大小选取合适的内存池分配内存
        // reinterpret_cast<T*>(...)：把 void* 强转为 T*
        if ((p = reinterpret_cast<T*>(HashBucket::useMemory(sizeof(T)))) != nullptr)
        {
            // 把 args 参数原封不动地转发给 T 的构造函数，保证参数的左右值属性不变（完美转发）
            // 不分配新内存，直接在 p 指向的那块已经分配好的内存上调用 T 的构造函数
            new(p) T(std::forward<Args>(args)...);
        }

        return p;
    }

    template<typename T>
    // 先析构对象，再把内存还给内存池
    void deleteElement(T* p)
    {
        if (p)
        {
            p->~T(); // 对象析构
            HashBucket::freeMemory(reinterpret_cast<void*>(p), sizeof(T)); // 内存回收
        }
    }
}
