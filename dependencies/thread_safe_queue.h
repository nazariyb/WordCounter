#ifndef TREAD_POOL_THREAD_SAFE_QUEUE_H
#define TREAD_POOL_THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class thread_safe_queue
    {
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    thread_safe_queue () = default;

    void push (T new_value)
    {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(std::move(new_value));
        data_cond.notify_one();
    }

    void double_push(T first, T second)
    {
        push(first);
        push(first);
    }

    void wait_and_pop (T &value)
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        value = std::move(data_queue.front());
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop ()
    {
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] { return !data_queue.empty(); });
        std::shared_ptr<T> res(
                std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    bool try_pop (T &value)
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return false;
        value = std::move(data_queue.front());
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop ()
    {
        std::lock_guard<std::mutex> lk(mut);
        if (data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(
                std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }

    std::pair<T, T> double_pop()
    {

        auto first = wait_and_pop().get();
        auto second = wait_and_pop().get();

        return std::make_pair(*first, *second);
    }

    unsigned size()
    {
        return 2;
    }

    bool empty () const
    {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }
    };


#endif //TREAD_POOL_THREAD_SAFE_QUEUE_H
