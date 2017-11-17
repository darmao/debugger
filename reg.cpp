//
// Created by darmao on 17-11-13.
//
#include <sys/user.h>
#include <sys/ptrace.h>
#include "reg.h"
uint64_t get_register_value(pid_t pid,reg r)
{
    user_regs_struct regs;
    //这里父进程将子进程所有寄存器的值都放在了regs结构体里面
    ptrace(PTRACE_GETREGS,pid,nullptr,&regs);
    //获取我们传进去的寄存器在regs中的偏移
    auto it=std::find_if(begin(g_register_descriptors),end(g_register_descriptors),[r](reg_descriptor rd){
        return rd.r==r;
    });
    /**
     * 用存放了子进程reg结构体内容的regs获取我们制定的寄存器的值,下面这样代码首先将regs的地址强转成
     * 一个指向uint64_t的指针，这个指针加1，就相当于偏移了sizeof(uint64_t)，这里偏移了it-begin(g_register_descriptors)，
     * 也就是我们需要查看的寄存器
     */

    return *(reinterpret_cast<uint64_t*>(&regs)+(it-begin(g_register_descriptors)));
}

void set_register_value(pid_t pid,reg r,uint64_t value)
{
    user_regs_struct regs;
    ptrace(PTRACE_GETREGS,pid, nullptr,&regs);
    auto it=std::find_if(begin(g_register_descriptors),end(g_register_descriptors),[r](reg_descriptor rd){
        return rd.r==r;
    });
    *(reinterpret_cast<uint64_t*>(&regs)+(it-begin(g_register_descriptors)))=value;
    ptrace(PTRACE_SETREGS,pid, nullptr,&regs);
}

uint64_t get_register_value_from_dwarf_register(pid_t pid, unsigned regnum)
{
    user_regs_struct regs;
    auto it=std::find_if(begin(g_register_descriptors),end(g_register_descriptors),[regnum](reg_descriptor rd){
        return rd.dwarf_r==regnum;
    });
    if(it==end(g_register_descriptors))
    {
        throw std::out_of_range{"Unknow dwarf register"};
    }
    ptrace(PTRACE_GETREGS,pid, nullptr,&regs);
    return *(reinterpret_cast<uint64_t*>(&regs)+(it-begin(g_register_descriptors)));
}

std::string get_register_name(reg r)
{
    auto it=std::find_if(begin(g_register_descriptors),end(g_register_descriptors),[r](reg_descriptor rd){
       return rd.r==r;
    });
    return it->name;

}
reg get_register_from_name(const std::string&name)
{
    auto it=std::find_if(begin(g_register_descriptors),end(g_register_descriptors),[name](reg_descriptor rd){
        return rd.name==name;
    });
    return it->r;
}