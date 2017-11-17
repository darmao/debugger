//
// Created by darmao on 17-11-13.
//
#include <sstream>
#include "utils.h"
std::vector<std::string>split(const std::string &s,char delimiter)//使用delimiter将s分开
{
    std::vector<std::string>out{};
    std::stringstream ss{s};
    std::string item;
    while(std::getline(ss,item,delimiter))
    {
        out.push_back(item);
    }
    return out;
}
bool is_prefix(const std::string& s,const std::string& of)//判断s是否是of的前缀
{
    if(s.size()>of.size())return false;
    return std::equal(s.begin(),s.end(),of.begin());
}

