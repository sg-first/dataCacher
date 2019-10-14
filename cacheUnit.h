#pragma once
#include "IOinter.h"
#include <tuple>

template<class dataType> class fullyAssoUnit;
template<class dataType> class dirctUnit;

template<class dataType>
class unit
{
private:
    friend class fullyAssoUnit<dataType>;
    friend class dirctUnit<dataType>;
    bool isEmpty=true;
    dataType content;

public:
    unit(uint IOtag) : IOtag(IOtag) {}
    uint IOtag;
    uint wordAddress;
    bool notUpdateContent=false;

    bool getIsEmpty() { return this->isEmpty; }
    virtual dataType read() { return this->content; }
    virtual void write(dataType contetn)=0; //写入外存

    virtual void update(uint blockNum, uint wordAddress,dataType content)
    {
        //以下非完整行为，需子类补全
        if(this->notUpdateContent) //有content缓存标记自动写回
            this->write(this->content);
    }

    void setContent(dataType content)
    {
        isEmpty=false;
        this->content=content;
    }
};


template<class dataType>
class fullyAssoUnit : public unit<dataType>
{
public:
    fullyAssoUnit(uint IOtag) : unit<dataType>(IOtag) {}
    uint blockNum;
    uint hitCount;

    virtual void write(dataType content)
    {
        this->setContent(content);
        IO::write(this->blockNum,this->wordAddress,content,this->IOtag);
    }

    virtual void update(uint blockNum, uint wordAddress, dataType content)
    {
        unit<dataType>::update(blockNum,wordAddress,content);
        this->blockNum=blockNum;
        this->wordAddress=wordAddress;
        unit<dataType>::setContent(content);
    }
};


template<class dataType>
class dirctUnit : public unit<dataType>
{
public:
    dirctUnit(uint IOtag) : unit<dataType>(IOtag) {}

    static string zeroCompensation(string sn)
    {
        while(sn.size()<totDight)
            sn="0"+sn;
        return sn;
    }
    static string zeroCompensation(uint n)
    {
        string sn=to_string(n);
        return zeroCompensation(sn);
    }
    static tuple<uint,uint> getTagAndLine(uint blockName)
    {
        string strBlockNum=dirctUnit<dataType>::zeroCompensation(blockName);
        uint tag=stoi(strBlockNum.substr(dirctUnit<dataType>::totDight-dirctUnit<dataType>::tagDigit));
        uint lineNum=stoi(strBlockNum.substr(0,dirctUnit<dataType>::totDight-dirctUnit<dataType>::tagDigit));
        return make_tuple(tag,lineNum);
    }

    const static uint tagDigit=1;
    const static uint totDight=2;
    uint tag;
    uint lineNum;

    virtual void write(dataType content)
    {
        this->setContent(content);
        uint blockNum=stoi(to_string(lineNum)+to_string(tag));
        IO::write(blockNum,this->wordAddress,content,this->IOtag);
    }

    virtual void update(uint blockNum, uint wordAddress, dataType content)
    {
        unit<dataType>::update(blockNum,wordAddress,content);
        tie(this->tag,this->lineNum)=getTagAndLine(blockNum);
        this->wordAddress=wordAddress;
        unit<dataType>::setContent(content);
    }
};
