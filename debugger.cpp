//
// Created by darmao on 17-10-30.
//
#include <wait.h>
#include "debugger.h"
#include "linenoise.h"
#include "utils.h"
#include <sys/ptrace.h>
#include <sys/user.h>
#include <iomanip>
#include <fstream>
#include <bits/siginfo.h>
#include "reg.h"

void debugger::run()
{
    int wait_status;
    auto options=0;
    waitpid(m_pid,&wait_status,options);//父进程阻塞在这里等待子进程启动完毕,此时没有父进程重新启动的的命令，子进程挂起
    char* line= nullptr;
    while((line=linenoise("(myGDB)"))!= nullptr)
    {
        handle_command(line);
        linenoiseHistoryAdd(line);
        linenoiseFree(line);
    }

}

void debugger::handle_command(const  std::string&line)
{

    auto args=split(line,' ');
    auto command=args[0];
    if(is_prefix(command,"continue"))
    {
        continue_execution();
    }else if(is_prefix(command,"break"))
    {
        std::string addr{args[1],2};//从0x以后开始初始地址,认为用户已经加上了0x
        set_breakpoint_at_address(std::stol(addr,0,16));//设置断点
    }else if(is_prefix(command,"register"))
    {
        if(is_prefix(args[1],"dump"))
        {
            dump_register();
        }else if(is_prefix(args[1],"read"))
        {
            std::cout<<get_register_value(m_pid,get_register_from_name(args[2]))<<std::endl;

        }else if(is_prefix(args[1],"write"))
        {
            std::string val{args[3],2};
            set_register_value(m_pid,get_register_from_name(args[2]),std::stol(val,0,16));
        }
    }else if(is_prefix(command,"memory"))
    {
        std::string addr{args[2],2};
        if(is_prefix(args[1],"read"))
        {
            std::cout<<std::hex<<read_memory(std::stol(addr,0,16))<<std::endl;
        }
        if(is_prefix(args[1],"write"))
        {
            std::string val{args[3],2};
            write_memory(std::stol(addr,0,16),std::stol(val,0,16));
        }
    }
    else
    {
        std::cerr<<"Unknow command\n";
    }
}
void debugger::continue_execution()
{
    /*ptrace(PTRACE_CONT,m_pid, nullptr, nullptr);//重新运行
    int wait_status;
    auto options=0;
    waitpid(m_pid,&wait_status,options);//在这里重新等待子进程的状态改变*/
    step_over_breakpoint();//单步调试
    ptrace(PTRACE_CONT,m_pid,nullptr, nullptr);//被调试进程执行完一条指令退出step_over函数后，父进程继续让其运行，
    wait_for_signal();//子进程没有运行到下一个断点的时候，父进程阻塞在这里，当运行到下一个断点是偶，触发int3断点，父进程解除阻塞，跳出hand_command函数
}
void debugger::set_breakpoint_at_address(std::intptr_t addr)//设置断点
{
    std::cout<<"Set breakpoint at address 0x"<<std::hex<<addr<<endl;
    breakpoint bp{m_pid,addr};
    bp.enable();
    m_breakpoints.insert(make_pair(addr,bp));
    //m_breakpoints[addr]=bp;c++14可以这么写

}
void debugger::dump_register()
{
    for(const auto&rd:g_register_descriptors)
    {
        std::cout<<rd.name<<"0x"<<std::setfill('0')<<std::setw(16)<<std::hex<<get_register_value(m_pid,rd.r)<<std::endl;
    }
}
uint64_t debugger::read_memory(uint64_t address)
{
    return ptrace(PTRACE_PEEKDATA,m_pid,address,nullptr);
}
void debugger::write_memory(uint64_t address, uint64_t value)
{
    ptrace(PTRACE_POKEDATA,m_pid,address,value);
}
uint64_t debugger::get_pc()
{
    return get_register_value(m_pid,reg::rip);
}
void debugger::set_pc(uint64_t val)
{
    set_register_value(m_pid,reg::rip,val);
}

void debugger::step_over_breakpoint()
{
    auto possible_breakpoint_location=get_pc()-1;//uint64_t data_with_uint3=((data&~0xff)|int3);//最低字节改为0xcc，最低字节
                                                 //高地址，和字节序有关

    if(m_breakpoints.count(possible_breakpoint_location))//如果当前地址是断点
    {
       // auto &bp=m_breakpoints[possible_breakpoint_location];
        auto &bp=m_breakpoints.at(possible_breakpoint_location);//得到当前这个位置上的breakpoint对象
        if(bp.is_enable())//如果是断点的话会返回true
        {
            auto previous_instruction_address=possible_breakpoint_location;
            set_pc(previous_instruction_address);//pc指针指向当前地址
            bp.disable();//恢复当前地址被修改成int3的数据
            ptrace(PTRACE_SINGLESTEP,m_pid, nullptr,nullptr);//重启子进程的执行，但指定子进程在下个入口或从系统调用退出时，
                                                            // 或者执行单个指令后停止执行，这可用于实现单步调试
            wait_for_signal();//调试进程阻塞在这里，当子进程执行单个指令后退出时候，状态会被调试进程收到，调试进程解除阻塞，退出step_over_break_point
            bp.enable();//加入int3断点
        }
    }
}

void debugger::wait_for_signal()
{
    int wait_status;
    auto options=0;
    waitpid(m_pid,&wait_status,options);//父进程阻塞，等待子进程触发一个int3断点从而发送一个sigtrap信号

    auto siginfo=get_signal_info();//得到收到的信号
    switch (siginfo.si_signo)
    {
        case SIGTRAP: //如果是我们关心的int3断点的信号进行处理
            handle_sigtrap(siginfo);
            break;
        case SIGSEGV:
        std::cout<<"Yay,segfault.Reason:"<<siginfo.si_code<<std::endl;
            break;
        default:
        std::cout<<"Got signal"<<strsignal(siginfo.si_signo)<<std::endl;

    }

}


dwarf::die debugger::get_function_from_pc(uint64_t pc)
{
    for(auto &cu:m_dwarf.compilation_units())
    {
        if(die_pc_range(cu.root()).contains(pc))
        {
            for(const auto& die:cu.root())
            {
                if(die.tag==dwarf::DW_TAG::subprogram)
                {
                    if(die_pc_range(die).contains(pc))
                    {
                        return die;
                    }
                }
            }
        }
    }
}
dwarf::line_table::iterator debugger::get_line_entry_from_pc(uint64_t pc)
{
    for(auto&cu:m_dwarf.compilation_units())
    {
        if((die_pc_range(cu.root())).contains(pc))
        {
            auto &lt=cu.get_line_table();
            auto it=lt.find_address(pc);
            if(it==lt.end())
            {
                throw std::out_of_range{"Cannot find line entry"};
            } else
            {
                return it;
            }
        }
    }
    throw std::out_of_range{"Cannot find line entry"};
}
void debugger::print_source(const std::string &file_name, unsigned line, unsigned n_lines_context)
{
    std::ifstream file{file_name};
    auto start_line=line<=n_lines_context?1:line-n_lines_context;
    auto end_line=line+n_lines_context+(line<n_lines_context?n_lines_context-line:0)+1;
    char c{};
    auto current_line=1u;
    while(current_line!=start_line&&file.get(c))
    {
        if(c=='\n')
        {
            ++current_line;
        }
    }
    std::cout<<(current_line==line?">":" ");
    while(current_line<=end_line&&file.get(c))
    {
        std::cout<<c;
        if(c=='\n')
        {
            ++current_line;
            std::cout<<(current_line==line?">":" ");
        }
    }
    std::cout<<std::endl;

}

siginfo_t debugger::get_signal_info()
{
    siginfo_t info;
    ptrace(PTRACE_GETSIGINFO,m_pid,nullptr,&info);
    return info;
}

void debugger::handle_sigtrap(siginfo_t info)
{
    switch (info.si_code)
    {
        case SI_KERNEL:
        case TRAP_BRKPT:
        {
            set_pc(get_pc()-1);//pc指针指向当前指令
            std::cout<<"Hit breakpoint at address 0X"<<std::hex<<get_pc()<<std::endl;
            auto line_entry=get_line_entry_from_pc(get_pc());
            print_source(line_entry->file->path,line_entry->line,0);//打印资源
            set_pc(get_pc()+1);//恢复pc指针指向当前指令的下一条指令
            return ;
        }
        case TRAP_TRACE:
            return;
        default:
        std::cout<<"Unknown GIFTRAP code"<<info.si_code<<std::endl;
            return;

    }
}





