#pragma once
#include "20210406_tradeapi_se_windows/ThostFtdcUserApiStruct.h"
#include "reporter.hpp"
#include <map>
#include <string>
#include <vector>

/////////////////////////////
// 数据处理器
/////////////////////////////

// 处理器基类
class handler_base
{
public:
    virtual ~handler_base(){};

    // 处理
    virtual void handle(const CThostFtdcDepthMarketDataField *data) = 0;

    // 返回处理器名字
    virtual std::string name() = 0;
};

// 原始数据->本地文件
class file_keeper : public handler_base
{
private:
    file_keeper(){};
    file_keeper(const file_keeper &) = delete;            // 禁止拷贝构造函数
    file_keeper &operator=(const file_keeper &) = delete; // 禁止赋值运算符
protected:
    // 输出流
    std::map<std::string, std::ofstream *> _files;

public:
    static file_keeper *getInstance();
    virtual std::string name() override;
    virtual void handle(const CThostFtdcDepthMarketDataField *data) override;
    virtual ~file_keeper();
};

// 原始数据->内存
class mem_keeper : public handler_base
{
private:
    mem_keeper(){};
    mem_keeper(const mem_keeper &) = delete;            // 禁止拷贝构造函数
    mem_keeper &operator=(const mem_keeper &) = delete; // 禁止赋值运算符
protected:
    std::map<std::string, std::vector<CThostFtdcDepthMarketDataField>> _datas;

public:
    static mem_keeper *getInstance();
    virtual std::string name() override;
    virtual void handle(const CThostFtdcDepthMarketDataField *data) override;
};

// 处理器汇总
class handles
{
protected:
    reporter_md reporter;

    // 处理器集合
    std::map<std::string, handler_base *> _handles;

public:
    // 更新数据
    void update(const CThostFtdcDepthMarketDataField *data);

    // 增加处理器
    void add_handler(handler_base *handler);

    // 删除处理器
    void remove_handler(std::string handle_name);
};