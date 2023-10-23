#pragma once
#include "20210406_tradeapi_se_windows/ThostFtdcTraderApi.h"
#include "20210406_tradeapi_se_windows/ThostFtdcMdApi.h"
#include <string>
#include <iostream>
#include <mutex>
#include <ctime>
#include <fstream>
#include <io.h>
#include <direct.h>

///////////////////////////////
// 负责输出信息
///////////////////////////////

class reporter_base
{
private:
    // 线程互斥量
    std::mutex writeMutex;

protected:
    // 文件流
    std::ofstream _file;

    // 写入计数
    int _write_count = 0;

    // 文件名
    std::string filename;

public:
    reporter_base()
    {
        // 检查路径
        if (_access("log", 0) == -1)
        {
            _mkdir("log");
        }

        std::time_t currentTime = std::time(nullptr);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y%m%d-%H%M%S", std::localtime(&currentTime));

        // 文件名 log-20210406-093000.txt
        filename = std::string("log/") + buffer + ".txt";
        _file.open(filename.data(), std::ios::app);
    };

    virtual ~reporter_base()
    {
        // 关闭文件
        if (_file.is_open())
            _file.close();
    };

    // 返回类型名称
    // [Md]
    // [Trade]
    virtual std::string name() = 0;

    void report(const std::string &msg)
    {
        // 锁
        std::lock_guard<std::mutex> lock(writeMutex);
        std::cout << name() << msg << std::endl;

        _file << name() << msg << std::endl;
        _write_count++;

        // 每100条保存重开文件
        if (_write_count % 100 == 0)
        {
            _file.close();
            _file.open(filename.data(), std::ios::app);
        }
    }

    void report_send_ret(const int ret)
    {
        // 0: 发送成功
        // -1: 因网络原因发送失败
        // -2: 未处理请求队列总数量超限。
        // -3: 每秒发送请求数量超限。

        std::string msg = "";
        switch (ret)
        {
        case 0:
            msg = "发送成功";
            break;

        case -1:
            msg = "因网络原因发送失败";
            break;
        case -2:
            msg = "未处理请求队列总数量超限";
            break;

        case -3:
            msg = "每秒发送请求数量超限";
            break;
        }

        report(msg);
    }

    // 回报回应信息
    bool report_response(CThostFtdcRspInfoField *pRspInfo)
    {
        // 如果ErrorID != 0, 说明收到了错误的响应
        bool res = pRspInfo && (pRspInfo->ErrorID == 0);
        if (!res && pRspInfo)
        {
            std::string msg = std::string("ErrorID=") + std::to_string(pRspInfo->ErrorID) + ", ErrorMsg=" + pRspInfo->ErrorMsg;
            report(msg);
        }
        return res;
    }
};

class reporter_md : public reporter_base
{
public:
    // 返回类型名称
    virtual std::string name() { return "[Md]"; }
};

class reporter_trade : public reporter_base
{
public:
    // 返回类型名称
    virtual std::string name() { return "[Trade]"; }
};
