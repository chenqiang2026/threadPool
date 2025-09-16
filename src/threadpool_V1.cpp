#include "threadpool_V1.h"

namespace opensource {
namespace cpp {
ThreadPool::ThreadPool()
    : threadNum_(1), bTerminate_(false)
{
}
ThreadPool::~ThreadPool()
{
    terminate();
}

bool ThreadPool::init(size_t num)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (!threads_.empty())
    {
        return false;
    }

    threadNum_ = num;
    return true;
}

void ThreadPool::terminate() {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        bTerminate_ = true;
        condition_.notify_all();
    }
    for (size_t i = 0; i < threads_.size(); i++)
    {
        if(threads_[i].joinable())
        {
            threads_[i].join();
        }
    }
    std::unique_lock<std::mutex> lock(mutex_);
    threads_.clear();
}

bool ThreadPool::start()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (!threads_.empty())
    {
        return false;
    }
    for (size_t i = 0; i < threadNum_; i++)
    {
        worker_= thread(&ThreadPool::run, this);
        //2种方法都可以
        //threads_.push_back(std::move(worker_));
        threads_.emplace_back(std::move(worker_));
    }
    return true;
}

bool ThreadPool::get(TaskFuncPtr& task)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (tasks_.empty())
    {
        condition_.wait(lock, [this] { return bTerminate_
                    || !tasks_.empty();
        });
    }
    if (bTerminate_)
        return false;
    if (!tasks_.empty())
    {
        task = std::move(tasks_.front());
        tasks_.pop();
        return true;
    }
    return false;
}

void ThreadPool::run()
{
    while (!isTerminate())
    {
        TaskFuncPtr task;
        bool ok = get(task);
        if (ok)
        {
            ++atomic_;
            try
            {
                if (task->_expireTime != 0 && task->_expireTime  < TNOWMS )
                {
                    //超时任务，是否需要处理?
                }
                else
                {
                    task->_func();
                }
            }
            catch (std::exception &e)
            {
                throw;
            }
            --atomic_;
            std::unique_lock<std::mutex> lock(mutex_);
            if (atomic_ == 0 && tasks_.empty())
            {
                condition_.notify_all();
            }
        }
    }
}
// 1000ms
bool ThreadPool::waitForAllDone(int millsecond)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (tasks_.empty())
        return true;
    if (millsecond < 0)
    {
        condition_.wait(lock, [this] { return tasks_.empty(); });
        return true;
    }
    else
    {
        return condition_.wait_for(lock, std::chrono::milliseconds(millsecond), [this] { return tasks_.empty(); });
    }
}

int gettimeofday(struct timeval &tv)
{
#if WIN32
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year   = wtm.wYear - 1900;
    tm.tm_mon   = wtm.wMonth - 1;
    tm.tm_mday   = wtm.wDay;
    tm.tm_hour   = wtm.wHour;
    tm.tm_min   = wtm.wMinute;
    tm.tm_sec   = wtm.wSecond;
    tm. tm_isdst  = -1;
    clock = mktime(&tm);
    tv.tv_sec = clock;
    tv.tv_usec = wtm.wMilliseconds * 1000;

    return 0;
#else
    return ::gettimeofday(&tv, 0);
#endif
}

void ThreadPool::getNow(timeval *tv)
{
#if TARGET_PLATFORM_IOS || TARGET_PLATFORM_LINUX
    int idx = _buf_idx;
    *tv = _t[idx];
    if(fabs(_cpu_cycle - 0) < 0.0001 && _use_tsc)
    {
        addTimeOffset(*tv, idx);
    }
    else
    {
        TC_Common::gettimeofday(*tv);
    }
#else
    gettimeofday(*tv);
#endif
}

int64_t ThreadPool::getNowMs()
{
    struct timeval tv;
    getNow(&tv);
    return tv.tv_sec * (int64_t)1000 + tv.tv_usec / 1000;
}
} // namespace opensource
} // namespace cpp
