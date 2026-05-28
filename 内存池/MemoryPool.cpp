#include "MemoryPool.h"

namespace Kama_memoryPool
{
    // 构造函数（用到初始化参数列表）
    MemoryPool::MemoryPool(size_t BlockSize)
        : BlockSize_(BlockSize)
        , SlotSize_(0)
        , firstBlock_(nullptr)
        , curSlot_(nullptr)
        , freeList_(nullptr)
        , lastSlot_(nullptr)
    {
    }

    // 程序退出时，把内存池申请的所有大块内存，全部还给系统，防止内存泄漏
    MemoryPool::~MemoryPool()
    {
        // 把连续的block删除
        Slot* cur = firstBlock_;
        while (cur)
        {
            Slot* next = cur->next;
            // 等同于 free(reinterpret_cast<void*>(firstBlock_));
            // 转化为 void 指针：因为 void 类型不需要调用析构函数，只释放空间
            operator delete(reinterpret_cast<void*>(cur));
            cur = next;
        }
    }

    // 给一个内存池设置好槽大小，并重置所有状态
    void MemoryPool::init(size_t size)
    {
        assert(size > 0);     // 断言检查：如果传了 0，程序会直接报错
        SlotSize_ = size;
        firstBlock_ = nullptr;
        curSlot_ = nullptr;
        freeList_ = nullptr;
        lastSlot_ = nullptr;
    }

    // 分配槽
    void* MemoryPool::allocate()
    {
        // 优先使用空闲链表中的内存槽
        Slot* slot = popFreeList();
        if (slot != nullptr)
        {
            return slot;
        }

        Slot* temp;
        // 此处大括号用于限定锁的范围
        // lock_guard：RAII 自动锁，构造时自动加锁，析构时自动解锁
        {
            std::lock_guard<std::mutex> lock(mutexForBlock_);
            if (curSlot_ >= lastSlot_)
            {
                // 当前内存块已无内存槽可用，开辟一块新的内存
                allocateNewBlock();
            }

            temp = curSlot_;
            // 这里不能直接 curSlot_ += SlotSize_ 因为 curSlot_ 是 Slot* 类型
            curSlot_ += SlotSize_ / sizeof(Slot);
        }

        return temp;
    }

    // 回收槽到空闲槽链表
    void MemoryPool::deallocate(void* ptr)
    {
        if (!ptr)
        {
            return;
        }

        Slot* slot = reinterpret_cast<Slot*>(ptr);
        pushFreeList(slot);
    }

    // 申请新的大块内存
    void MemoryPool::allocateNewBlock()
    {
        // 头插法插入新的大内存块
        void* newBlock = operator new(BlockSize_);
        reinterpret_cast<Slot*>(newBlock)->next = firstBlock_;
        firstBlock_ = reinterpret_cast<Slot*>(newBlock);

        char* body = reinterpret_cast<char*>(newBlock) + sizeof(Slot*);
        size_t paddingSize = padPointer(body, SlotSize_);  // 计算对齐需要填充内存的大小
        curSlot_ = reinterpret_cast<Slot*>(body + paddingSize);

        // 计算新申请大内存块的尾部边界
        lastSlot_ = reinterpret_cast<Slot*>(reinterpret_cast<size_t>(newBlock) + BlockSize_ - SlotSize_ + 1);
    }

    // 让指针对齐到槽大小的倍数位置
    size_t MemoryPool::padPointer(char* p, size_t align)
    {
        // align 是槽大小
        return align - (reinterpret_cast<size_t>(p) % align);
    }

    // 实现无锁入队操作
    bool MemoryPool::pushFreeList(Slot* slot)
    {
        while (true)
        {
            // 读取空闲链表的头节点地址
            Slot* oldHead = freeList_.load(std::memory_order_relaxed);
            // 将新节点的 next 指向当前头节点
            slot->next.store(oldHead, std::memory_order_relaxed);

            // 尝试将新节点设置为头节点
            if (freeList_.compare_exchange_weak(oldHead, slot, std::memory_order_release, std::memory_order_relaxed))
            {
                return true;
            }
            // 失败则说明另一个线程可能已经修改了 freeList_，CAS 需要重试
        }
    }

    // 实现无锁出队操作
    Slot* MemoryPool::popFreeList()
    {
        while (true)
        {
            Slot* oldHead = freeList_.load(std::memory_order_acquire);
            if (oldHead == nullptr)
            {
                return nullptr;      // 队列为空
            }

            // 在访问 newHead 之前再次验证 oldHead 的有效性
            Slot* newHead = nullptr;
            try
            {
                newHead = oldHead->next.load(std::memory_order_relaxed);
            }
            catch (...)
            {
                
                continue;          // 失败则重新尝试申请内存
            }

            // 尝试更新头结点
            if (freeList_.compare_exchange_weak(oldHead, newHead, std::memory_order_acquire, std::memory_order_relaxed))
            {
                return oldHead;    // 把 oldHead 拆下来用
            }
            // 失败则说明 oldHead 可能被其他线程先拆走了，CAS 需要重试
        }
    }

    // 初始化分级内存池
    void HashBucket::initMemoryPool()
    {
        for (int i = 0; i < MEMORY_POOL_NUM; i++)
        {
            getMemoryPool(i).init((i + 1) * SLOT_BASE_SIZE);
        }
    }

    // 单例模式
    MemoryPool& HashBucket::getMemoryPool(int index)
    {
        static MemoryPool memoryPool[MEMORY_POOL_NUM];
        return memoryPool[index];
    }
}

