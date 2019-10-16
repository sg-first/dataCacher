#pragma once
#include <string>
using namespace std;

typedef unsigned int uint;

//可根据底层数据制式修改本类中函数
//目前dataType=string，block为配置文件，word为配置文件中变量
class IO
{
public:
    static string read(uint blockNum, uint wordAddress, uint IOtag)
    {
        string path=to_string(blockNum);
        string varName=to_string(wordAddress);
    }

    static void write(uint blockNum,uint wordAddress, string content, uint IOtag)
    {
        string path=to_string(blockNum);
        string varName=to_string(wordAddress);
    }
};
