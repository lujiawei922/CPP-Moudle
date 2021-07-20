#ifndef DEF_threadsafe_queue
#define DEF_threadsafe_queue


#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
template<typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue() {}
    void push(T new_value)//入队操作
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }

    T wait_and_pop()//直到有元素可以删除为止，与WakeUp配合使用
    {
        std::unique_lock<std::mutex> lk(mut);
        if (data_queue.empty())
        {
            data_cond.wait(lk);
        }
        if (data_queue.empty())
            return NULL;
        T res = data_queue.front();
        if (NULL != res)
            data_queue.pop();
        return res;
    }

    T try_pop()//不管有没有队首元素直接返回
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return NULL;
        T res = data_queue.front();
        if (NULL != res)
            data_queue.pop();
        return res;
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
    size_t count() const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.size();
    }
    void WakeUp()
    {
        data_cond.notify_all();
    }
};

#endif
