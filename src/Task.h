#ifndef LAB4_TASK_H
#define LAB4_TASK_H

#include <qt5/QtCore/QThreadPool>
#include <atomic>

#include "../dependencies/thread_safe_queue.h"
#include "../dependencies/function_wrapper.h"
#include "main_config.h"

class Task : public QRunnable
    {
    public:
    function_wrapper func;

    explicit Task (function_wrapper& func_) : QRunnable() { func = std::move(func_); }

    void run () override { func(); };

    };


#endif //LAB4_TASK_H
