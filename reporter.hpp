#pragma once
#include "20210406_tradeapi_se_windows/ThostFtdcTraderApi.h"
#include "20210406_tradeapi_se_windows/ThostFtdcMdApi.h"
#include <string>
#include <iostream>
#include <mutex>

///////////////////////////////
// 负责输出信息
///////////////////////////////

// 线程互斥量
extern std::mutex actMutex;

class reporter_base
{
protected:
    int requestID = 0;

public:
    virtual ~reporter_base(){};

    // 返回类型名称
    // [Md]
    // [Trade]
    virtual std::string name() = 0;

    void report(const std::string &msg)
    {
        // 锁
        std::lock_guard<std::mutex> lock(actMutex);
        std::cout << name() << msg << std::endl;
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
