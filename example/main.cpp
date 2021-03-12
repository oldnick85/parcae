#include <stdio.h>

#include "parcae.h"

static CParcaePtr g_parc = nullptr;
static int g_i = 0;
std::string str1;
std::string str2;

void func_parallel(const std::string &thread_name, const int n)
{
    g_parc->StartThread(thread_name);
    std::string &str = (n == 0) ? str1 : str2;
    g_parc->Milestone(thread_name, 0);
    g_i++;
    str.append(std::to_string(g_i));
    g_parc->Milestone(thread_name, 1);
    g_i++;
    str.append(std::to_string(g_i));
    g_parc->Milestone(thread_name, 2);
    g_parc->StopThread(thread_name);
}

void func()
{
    g_i = 0;
    str1.clear();
    str2.clear();
    std::thread t1(func_parallel, "A", 0);
    std::thread t2(func_parallel, "B", 1);
    t1.join();
    t2.join();
    printf("%s - %s\n", str1.c_str(), str2.c_str());
    g_parc->Stop();
}

int main()
{
    g_parc = CParcaePtr(new CParcae());
    g_parc->Start(func, {"B", "A"});
    g_parc = nullptr;
    return 0;
}
