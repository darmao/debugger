//
// Created by darmao on 17-11-13.
//
#include "breakpoint.h"
#include <sys/ptrace.h>
void breakpoint::enable()
{
    auto data=ptrace(PTRACE_PEEKDATA,m_pid,m_addr,nullptr);//从m_pid的m_addr出读取一个字作为调用的结果返回
    m_saved_data= static_cast<uint8_t>(data&0xff);//保存最低字节,这个字节在高位保存，所以rip-1就得到了设置0xcc地址的地方
    uint64_t int3=0xcc;//int3断点
    uint64_t data_with_uint3=((data&~0xff)|int3);//最低字节改为0xcc
    ptrace(PTRACE_POKEDATA,m_pid,m_addr,data_with_uint3);//将int3写入m_addr指向的写入内存
    m_enabled=true;
}
void breakpoint::disable()
{
    auto data=ptrace(PTRACE_PEEKDATA,m_pid,m_addr, nullptr);
    auto restored_data=((data&~0xff)|m_saved_data);//恢复m_addr指向被修改的int3断点
    ptrace(PTRACE_POKEDATA,m_pid,m_addr, restored_data);//将原来的数据写入m_addr这个地址
    m_enabled=false;
}
