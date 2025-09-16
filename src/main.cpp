#include <iostream>
#include <threadpool_V1.h>
using namespace std;
using namespace ::opensource::cpp;
void func0()
{
    cout << "func0()" << endl;
}

void func1(int a)
{
    cout << "func1 int =" << a << endl;
}

//void func1(string a)
//{
//    cout << "func1 string =" << a << endl;
//}

void func2(int a, string b)
{
    cout << "func2() a=" << a << ", b=" << b<< endl;
}


void test1() // 简单测试线程池
{
    ThreadPool threadpool;
    threadpool.init(1);
    threadpool.start();
//    threadpool.exec(1000,func0); // 1000是超时1000的意思，目前没有起作用
    threadpool.exec(func1, 10);
//    threadpool.exec((void(*)(int))func1, 10);
//    threadpool.exec((void(*)(string))func1, "1111");
    threadpool.exec(func2, 20, "darren");
    threadpool.waitForAllDone(); 
    threadpool.terminate();
}
int func1_future(int a)
{
    cout << "func1() a=" << a << endl;
    return a;
}

string func2_future(int a, string b)
{
    cout << "func1() a=" << a << ", b=" << b<< endl;
    return b;
}

void test2() // 测试任务函数返回值
{
    ThreadPool threadpool;
    threadpool.init(1);
    threadpool.start();
    std::future<decltype (func1_future(0))> result1 = threadpool.exec(func1_future, 10);
    std::future<string> result2 = threadpool.exec(func2_future, 20, "666");
//  auto result2 = threadpool.exec(func2_future, 20, "2222");
    std::cout << "result1: " << result1.get() << std::endl;
    std::cout << "result2: " << result2.get() << std::endl;
    threadpool.waitForAllDone();
    threadpool.terminate();
}

class Test
{
public:
    int test(int i){
        cout << _name << ", i = " << i << endl;
        return i;
    }
//    int test(string str) {
//        cout << _name << ", str = " << str << endl;
//    }
    void setName(string name){
        _name = name;
    }
    string _name;
};

void test3() // 测试类对象函数的绑定
{
    ThreadPool threadpool;
    threadpool.init(1);
    threadpool.start();
    Test t1;
    Test t2;
    t1.setName("Test1");
    t2.setName("Test2");
    auto f1 = threadpool.exec(std::bind(&Test::test, &t1, std::placeholders::_1), 10);
    auto f2 = threadpool.exec(std::bind(&Test::test, &t2, std::placeholders::_1), 20);
    threadpool.waitForAllDone();
    cout << "t1 " << f1.get() << endl;
    cout << "t2 " << f2.get() << endl;
}

void func2_1(int a, int b)
{
    cout << "func2_1 a + b = " << a+b << endl;
}

int func2_1(string a, string b)
{
    cout << "func2_1 a + b = " << a << b<< endl;
    return 0;
}
void test4() // 简单测试线程池
{
    ThreadPool threadpool;
    threadpool.init(1);
    threadpool.start();
    threadpool.exec((void(*)(int, int))func2_1, 10, 20);
    threadpool.exec((int(*)(string, string))func2_1, "aaa", "   bbb");
    threadpool.waitForAllDone(); 
    threadpool.terminate();
}

int main()
{
//    test1(); // 简单测试线程池
//    test2(); // 测试任务函数返回值
//    test3(); // 测试类对象函数的绑定
    test4();
    cout << "main finish!" << endl;
    return 0;
}
