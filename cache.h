#pragma once
#include "cacheUnit.h"
#include <cstdlib>
#include <ctime>

int getRandNum(int maxNum)
{
    return rand()%maxNum;
}

template<class dataType>
class cache
{
private:
    static const uint cacheSize=100;
    uint usedUnit=0;
    uint cacheType;
    //0为fullyAsso
    //1为dirct
    uint replacementStrategy;
    //0为LFU
    //1为LRU
    //2为随机替换
    uint writingStrategy;
    //0为写回法
    //1为全写法
    //2为写一次法（未考虑多cache共享外存同步问题，暂不支持）
    uint IOtag;
    unit<dataType>* cacheReservoir[cacheSize];

    fullyAssoUnit<dataType>* findFullyAssoUnit(uint blockNum, uint wordAddress)
    {
        for(uint ii=0; ii<this->usedUnit; ii++)
        {
            fullyAssoUnit<dataType>* i = this->cacheReservoir[ii];
            if(!i->getIsEmpty() && i->blockNum==blockNum && i->wordAddress==wordAddress)
            {
                //命中
                if(this->replacementStrategy==0)
                    i->hitCount++;
                else if(this->replacementStrategy==1)
                {
                    i->hitCount=0;
                    for(fullyAssoUnit<dataType>* j : this->cacheReservoir)
                    {
                        if(i!=j)
                            j->hitCount++;
                    }
                }
                return i;
            }
        }
        return nullptr;
    }

    dirctUnit<dataType>* findDirctUnit(uint blockNum, uint wordAddress)
    {
        uint tag,lineNum;
        tie(tag,lineNum) = dirctUnit<dataType>::getTagAndLine(blockNum);
        dirctUnit<dataType>* dui = (dirctUnit<dataType>*)this->cacheReservoir[tag]; //找到对应块
        //检测是否命中
        if(!dui->getIsEmpty() && dui->lineNum==lineNum && dui->wordAddress==wordAddress)
            return dui;
        else
            return nullptr;
    }

    fullyAssoUnit<dataType>* findMinMaxUnit(bool isMin=true)
    {
        uint hitCount;
        if(isMin)
            hitCount=UINT_MAX;
        else
            hitCount=0;
        fullyAssoUnit<dataType>* result;

        for(fullyAssoUnit<dataType>* i : this->cacheReservoir) //调用该函数说明在进行替换策略，cacheReservoir必定全满
        {
            if(!i->getIsEmpty())
            {
                if(isMin)
                {
                    if(i->hitCount<hitCount)
                    {
                        hitCount=i->hitCount;
                        result=i;
                    }
                }
                else
                {
                    if(i->hitCount>hitCount)
                    {
                        hitCount=i->hitCount;
                        result=i;
                    }
                }
            }
        }

        return result;
    }

    dataType externalMemoryRead(uint blockNum, uint wordAddress)
    {
        dataType content=IO::read(blockNum,wordAddress,this->IOtag);
        if(this->usedUnit<=cacheSize)
        {
            this->cacheReservoir[usedUnit]->update(blockNum,wordAddress,content);
            this->usedUnit++;
        }
        else
        {
            if(this->cacheType==0) //fullyAsso，使用替换策略
            {
                if(this->replacementStrategy==0) //LFU
                {
                    fullyAssoUnit<dataType>* minUnit=findMinMaxUnit();
                    minUnit->update(blockNum,wordAddress,content);
                    for(fullyAssoUnit<dataType>* i : this->cacheReservoir)
                        i->hitCount=0;
                }
                else if(this->replacementStrategy==1) //LRU
                {
                    fullyAssoUnit<dataType>* maxUnit=findMinMaxUnit(false);
                    maxUnit->update(blockNum,wordAddress,content);
                }
            }
            else //随机策略
            {
                uint sub=getRandNum(cacheSize);
                this->cacheReservoir[sub]->update(blockNum,wordAddress,content);
            }
        }
        return content;
    }

public:
    cache(uint cacheType, uint replacementStrategy, uint IOtag=0) :
        cacheType(cacheType), replacementStrategy(replacementStrategy), IOtag(IOtag)
    {
        srand(time(0));
        for(unit<dataType>* &i : this->cacheReservoir)
        {
            if(cacheType==0)
                i=new fullyAssoUnit<dataType>(this->IOtag);
            else if(cacheType==1)
                i=new dirctUnit<dataType>(this->IOtag);
        }
    }

    dataType read(uint blockNum, uint wordAddress)
    {
        unit<dataType>* i;
        if(this->cacheType==0)
            i = this->findFullyAssoUnit(blockNum,wordAddress);
        else if(this->cacheType==1)
            i = this->findDirctUnit(blockNum,wordAddress);

        if(i!=nullptr)
            return i->read();
        else
            return this->externalMemoryRead(blockNum,wordAddress);
    }

    void write(uint blockNum, uint wordAddress, dataType content)
    {
        unit<dataType>* i;
        if(this->cacheType==0)
            i = this->findFullyAssoUnit(blockNum,wordAddress);
        else if(this->cacheType==1)
            i = this->findDirctUnit(blockNum,wordAddress);

        if(i!=nullptr)
        {
            if(writingStrategy==0) //写回法
            {
                i->setContent(content);
                i->notUpdateContent=true;
            }
            else if(writingStrategy==1) //全写法
            {
                i->write(content);
            }
        }
        else
            IO::write(blockNum,wordAddress,content,this->IOtag); //fix:未命中的写不会更新cache，具体需不需要需要根据业务环境的IO特性调整
    }
};
