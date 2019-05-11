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

    void push (T new_value);

    void double_push (T first, T second);

    void wait_and_pop (T &value);

    std::shared_ptr<T> wait_and_pop ();

    bool try_pop (T &value);

    std::shared_ptr<T> try_pop ();

    std::pair<T, T> double_pop ();

    unsigned size () const;

    bool empty () const;
    };


#endif //TREAD_POOL_THREAD_SAFE_QUEUE_H
