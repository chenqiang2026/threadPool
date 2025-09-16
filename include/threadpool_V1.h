#ifndef THREADPOOL_V1_H
#define THREADPOOL_V1_H

#include <future>
#include <functional>
#include <iostream>
#include <queue>
#include <mutex>
#include <memory>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
using namespace std;
namespace opensource {
namespace cpp {

/////////////////////////////////////////////////
/**
 * @file .h
 * @brief 线程池类,采用c++11来实现了，
 * 使用说明:
 *  tpool;
 * tpool.init(5);   //初始化线程池线程数
 * //启动线程方式
 * tpool.start();
 * //将任务丢到线程池中
 * tpool.exec(testFunction, 10);    //参数和start相同
 * //等待线程池结束
 * tpool.waitForAllDone(1000);      //参数<0时, 表示无限等待(注意有人调用stop也会推出)
 * //此时: 外部需要结束线程池是调用
 * tpool.terminal();
 * 注意:
 * ThreadPool::exec执行任务返回的是个future, 因此可以通过future异步获取结果, 比如:
 * int testInt(int i)
 * {
 *     return i;
 * }
 * auto f = tpool.exec(testInt, 5);
 * cout << f.get() << endl;   //当testInt在线程池中执行后, f.get()会返回数值5
 *
 * class Test
 * {
 * public:
 *     int test(int i);
 * };
 * Test t;
 * auto f = tpool.exec(std::bind(&Test::test, &t, std::placeholders::_1), 10);
 * //返回的future对象, 可以检查是否执行
 * cout << f.get() << endl;
 */

class ThreadPool
{
protected:
    struct TaskFunc
    {
        TaskFunc(uint64_t expireTime) : _expireTime(expireTime)
        { }

        std::function<void()>   _func;
        int64_t                _expireTime = 0;	//超时的绝对时间
    };
    typedef shared_ptr<TaskFunc> TaskFuncPtr;
public:
    ThreadPool();
    virtual ~ThreadPool();
    bool init(size_t num);
    size_t getThreadNum()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        return threads_.size();
    }
    size_t getJobNum()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return tasks_.size();
    }
    void terminate();
    bool start(); // 创建线程
    /**
    * @brief 用线程池启用任务(F是function, Args是参数)
    *
    * @param ParentFunctor
    * @param tf
    * @return 返回任务的future对象, 可以通过这个对象来获取返回值
    */
    template <typename F, typename... Args>
    std::future<typename std::result_of<F(Args...)>::type> exec(F&& f, Args&&... args) 
    {
        return exec(0,f,args...);
    }
    /**
    * @brief 用线程池启用任务(F是function, Args是参数)
    *
    * @param 超时时间 ，单位ms (为0时不做超时控制) ；若任务超时，此任务将被丢弃
    * @param bind function
    * @return 返回任务的future对象, 可以通过这个对象来获取返回值
    */
    /*
    template <class F, class... Args>
    它是c++里新增的最强大的特性之一，它对参数进行了高度泛化，它能表示0到任意个数、任意类型的参数
    auto exec(F &&f, Args &&... args) -> std::future<decltype(f(args...))>
    std::future<decltype(f(args...))>：返回future，调用者可以通过future获取返回值
    返回值后置
    */
    template <typename F, typename... Args>
    std::future<typename std::result_of<F(Args...)>::type> exec(int64_t timeoutMs, F&& f, Args&&... args) 
    {
        int64_t expireTime =  (timeoutMs == 0 ? 0 : TNOWMS + timeoutMs);  // 获取现在时间
        //定义返回值类型
        using RetType = decltype(f(args...));  // 推导返回值
        // 封装任务
        auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        TaskFuncPtr fPtr = std::make_shared<TaskFunc>(expireTime);  // 封装任务指针，设置过期时间
        fPtr->_func = [task]() {  // 具体执行的函数
          (*task)();
        };

        std::unique_lock<std::mutex> lock(mutex_);
        tasks_.push(fPtr);              
        condition_.notify_one();        

        return task->get_future();;
    }
    bool waitForAllDone(int millsecond = -1);

protected:
    bool get(TaskFuncPtr&task);
    bool isTerminate() { return bTerminate_; }
    void run();
protected:
    queue<TaskFuncPtr> tasks_;
    std::vector<std::thread> threads_; 
    std::thread worker_;
    std::mutex    mutex_;
    std::condition_variable    condition_;
    size_t    threadNum_;
    bool    bTerminate_;
    std::atomic<int>    atomic_{ 0 };
private:
     void getNow(timeval *tv);
     int64_t getNowMs();
     const int64_t TNOWMS = getNowMs();
};
} // namespace cpp 
} // namespace opensource

#endif //THREADPOOL_V1_H
