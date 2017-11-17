//
// Created by darmao on 17-11-13.
//

#ifndef DEBUGGER_REG_H
#define DEBUGGER_REG_H

#include <string>
#include <boost/mpl/size_t.hpp>
#include <array>
#include <algorithm>
enum class reg
{
    rax,rbx,rcx,rdx,
    rdi,rsi,rbp,rsp,
    r8,r9,r10,r11,
    r12,r13,r14,r15,
    rip,eflags,cs,orig_rax,
    fs_base,gs_base,fs,gs,ss,ds,es
};
static constexpr std::size_t n_registers=27;
struct reg_descriptor
{
    reg r;
    int dwarf_r;
    std::string name;
};
const std::array<reg_descriptor,n_registers>g_register_descriptors
{{
        {reg::r15,0,"r15"},
        {reg::r14,1,"r14"},
        {reg::r13,2,"r13"},
        {reg::r12,3,"r12"},
        {reg::rbp,4,"rbp"},
        {reg::rbx,5,"rbx"},
        {reg::r11,6,"r11"},
        {reg::r10,7,"r10"},
        {reg::r9,8,"r9"},
        {reg::r8,9,"r8"},
        {reg::rax,10,"rax"},
        {reg::rcx,11,"rcx"},
        {reg::rdx,12,"rdx"},
        {reg::rsi,13,"rsi"},
        {reg::rdi,14,"rdi"},
        {reg::orig_rax,15,"orig_rax"},
        {reg::rip,16,"rip"},
        {reg::cs,17,"cs"},
        {reg::eflags,18,"eflags"},
        {reg::rsp,19,"rsp"},
        {reg::ss,20,"ss"},
        {reg::fs_base,21,"fs_base"},
        {reg::gs_base,22,"gs_base"},
        {reg::ds,23,"ds"},
        {reg::es,24,"es"},
        {reg::fs,25,"fs"},
        {reg::gs,19,"26"},

}};
uint64_t get_register_value(pid_t pid,reg r);
void set_register_value(pid_t,reg r,uint64_t value);
uint64_t get_register_value_from_dwarf_register(pid_t pid, unsigned regnum);
std::string get_register_name(reg r);
reg get_register_from_name(const std::string& name);


#endif //DEBUGGER_REG_H
