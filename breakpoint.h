//
// Created by darmao on 17-11-13.
//

#ifndef DEBUGGER_BREAKPOINT_H
#define DEBUGGER_BREAKPOINT_H

#include <cstdint>
#include <signal.h>

class breakpoint
{
public:
    breakpoint(pid_t pid,std::intptr_t addr) :m_pid{pid},m_addr{addr},m_enabled{false},m_saved_data{}
    {}
    void enable();
    void disable();
    auto is_enable()const->bool
    {
        return m_enabled;
    }
    auto get_address()const->std::intptr_t
    {
        return m_addr;
    }
private:
    pid_t m_pid;
    std::intptr_t m_addr;
    bool m_enabled;
    uint8_t m_saved_data;//存储断点的数据
};
#endif //DEBUGGER_BREAKPOINT_H
