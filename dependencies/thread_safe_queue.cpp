#include "thread_safe_queue.h"

//template<typename T>
//void thread_safe_queue<T>::push (T new_value)
//{
//    std::lock_guard<std::mutex> lk(mut);
//    data_queue.push(std::move(new_value));
//    data_cond.notify_one();
//}

//template<typename T>
//void thread_safe_queue<T>::double_push (T first, T second)
//{
//    push(first);
//    push(second);
//}

//template<typename T>
//void thread_safe_queue<T>::wait_and_pop (T &value)
//{
//    std::unique_lock<std::mutex> lk(mut);
//    data_cond.wait(lk, [this] { return !data_queue.empty(); });
//    value = std::move(data_queue.front());
//    data_queue.pop();
//}

//template<typename T>
//std::shared_ptr<T> thread_safe_queue<T>::wait_and_pop ()
//{
//    std::unique_lock<std::mutex> lk(mut);
//    data_cond.wait(lk, [this] { return !data_queue.empty(); });
//    std::shared_ptr<T> res(
//            std::make_shared<T>(std::move(data_queue.front())));
//    data_queue.pop();
//    return res;
//}

//template<typename T>
//bool thread_safe_queue<T>::try_pop (T &value)
//{
//    std::lock_guard<std::mutex> lk(mut);
//    if (data_queue.empty())
//        return false;
//    value = std::move(data_queue.front());
//    data_queue.pop();
//    return true;
//}
//
//template<typename T>
//std::shared_ptr<T> thread_safe_queue<T>::try_pop ()
//{
//    std::lock_guard<std::mutex> lk(mut);
//    if (data_queue.empty())
//        return std::shared_ptr<T>();
//    std::shared_ptr<T> res(
//            std::make_shared<T>(std::move(data_queue.front())));
//    data_queue.pop();
//    return res;
//}

//template<typename T>
//std::pair<T, T> thread_safe_queue<T>::double_pop ()
//{
//    auto first = wait_and_pop().get();
//    auto second = wait_and_pop().get();
//
//    return std::make_pair(*first, *second);
//}

//template<typename T>
//unsigned thread_safe_queue<T>::size () const
//{
//    std::lock_guard<std::mutex> lk(mut);
//    return data_queue.size();
//}

//template<typename T>
//bool thread_safe_queue<T>::empty () const
//{
//    std::lock_guard<std::mutex> lk(mut);
//    return data_queue.empty();
//}
