#ifndef LAB4_TASK_H
#define LAB4_TASK_H

#include <qt5/QtCore/QThreadPool>
#include <atomic>

#include "../dependencies/thread_safe_queue.h"
#include "main_config.h"

template<typename F>
class Task : public QRunnable
    {
    F func;

    explicit Task (F func_) : QRunnable(), func(func_) { }

    void run () override { func(); };
    };


#endif //LAB4_TASK_H
