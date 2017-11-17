//
// Created by darmao on 17-10-30.
//

#ifndef DEBUGGER_DEBUGGER_H
#define DEBUGGER_DEBUGGER_H

#include <iostream>
#include <unordered_map>
#include <fcntl.h>
#include "breakpoint.h"
#include "libelfin/dwarf/dwarf++.hh"
#include "libelfin/elf/elf++.hh"

using namespace std;
class debugger
{
public:
    debugger(std::string prog_name,pid_t pid):m_prog_name{move(prog_name)},m_pid{pid}
    {
        auto fd=open(m_prog_name.c_str(),O_RDONLY);
        m_elf=elf::elf{elf::create_mmap_loader(fd)};
        m_dwarf=dwarf::dwarf{dwarf::elf::create_loader(m_elf)};

    }
    void run();
    void handle_command(const std::string&line);
    void continue_execution();
    void set_breakpoint_at_address(std::intptr_t addr);
    void dump_register();
    uint64_t read_memory(uint64_t address);
    void write_memory(uint64_t address,uint64_t value);
    uint64_t get_pc();
    void set_pc(uint64_t val);
    void step_over_breakpoint();
    void wait_for_signal();
    dwarf::die get_function_from_pc(uint64_t pc);
    dwarf::line_table::iterator get_line_entry_from_pc(uint64_t pc);
    void print_source(const std::string& file_name, unsigned line, unsigned n_lines_context);
    siginfo_t get_signal_info();
    void handle_sigtrap(siginfo_t info);
private:
    string m_prog_name;
    pid_t m_pid;
    std::unordered_map<std::intptr_t ,breakpoint>m_breakpoints;
    dwarf::dwarf m_dwarf;
    elf::elf m_elf;

};
#endif //DEBUGGER_DEBUGGER_H
