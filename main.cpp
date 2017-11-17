#include <iostream>
#include <zconf.h>
#include<sys/ptrace.h>
#include "debugger.h"

int main(int argc,char *argv[]){
    using namespace std;
    if(argc<2)
    {
        cerr<<"Program name not specified"<<endl;
    }

    auto prog="/home/darmao/Desktop/algthorim";//要调试的程序的名字
    pid_t pid=fork();
    if(pid==0)
    {
        cout<<"子进程中"<<endl;
        ptrace(PTRACE_TRACEME,0, nullptr, nullptr);
        execl(prog,prog, nullptr);//执行要调试的程序，程序启动完毕会给父进程发送一个sigtrap信号
    }else if(pid>=1)
    {
        cout<<"父进程中"<<endl;
        debugger dbg{prog,pid};
        dbg.run();
    }



}