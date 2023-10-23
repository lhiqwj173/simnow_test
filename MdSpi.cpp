#include "MdSpi.h"
#include <iostream>
#include <vector>
#include <atomic>
#include <string>

extern CThostFtdcMdApi *Mdapi;
extern std::map<std::string, std::vector<std::string>> all_codes; // 可订阅代码 <交易所id: <代码id>>
extern std::vector<std::string> local_codes;
extern std::atomic<int> connect_flag;

// api线程互斥量
extern std::mutex actMutex;
extern std::condition_variable cv;
extern bool all_codes_done;

//////////////////////////
// 回调函数
//////////////////////////
void MdSpi::OnFrontConnected()
{
    reporter->report("连接建立");
    reporter->report("登入 ...");
    login();
}

void MdSpi::OnFrontDisconnected(int nReason)
{
    ///         0x1001 网络读失败
    ///         0x1002 网络写失败
    ///         0x2001 接收心跳超时
    ///         0x2002 发送心跳失败
    ///         0x2003 收到错误报文
    std::string msg = "";
    switch (nReason)
    {
    case 0x1001:
        msg = "网络读失败";
        break;
    case 0x1002:
        msg = "网络写失败";
        break;
    case 0x2001:
        msg = "接收心跳超时";
        break;
    case 0x2002:
        msg = "发送心跳失败";
        break;
    case 0x2003:
        msg = "收到错误报文";
        break;
    }

    reporter->report("连接断开: " + msg);
}

void MdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    reporter->report_response(pRspInfo);
    connect_flag -= 1;
    Mdapi->Release();
}

void MdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (reporter->report_response(pRspInfo))
    {
        reporter->report("登入成功");
        reporter->report(std::string("交易日: ") + pRspUserLogin->TradingDay);

        connect_flag += 1;

        // 订阅数据
        subdata();
    }

    else
    {
        reporter->report("登入失败");
        Mdapi->Release();
    }
}

void MdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    // ErrorID=77, ErrorMsg=CTP:无此功能
    reporter->report("OnRspUserLogout");
    if (reporter->report_response(pRspInfo))
    {
        reporter->report("退登成功");
        connect_flag -= 1;
        Mdapi->Release();
    }
}

void MdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (reporter->report_response(pRspInfo))
    {
        reporter->report(std::string("订阅成功: ") + pSpecificInstrument->InstrumentID);

        // 记录当前订阅的品种
        cur_sub_codes.insert(pSpecificInstrument->InstrumentID);
    }
}

void MdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    // 更新数据处理
    reporter->report("");
    reporter->report(std::string("更新数据: ") + pDepthMarketData->InstrumentID + " " + pDepthMarketData->UpdateTime + " " + std::to_string(pDepthMarketData->UpdateMillisec));
    data_handles->update(pDepthMarketData);
}

void MdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (reporter->report_response(pRspInfo) && (pSpecificInstrument->InstrumentID)[0] != 0)
    {
        reporter->report(std::string("退订成功: ") + pSpecificInstrument->InstrumentID);

        // 更新当前订阅的品种
        if (cur_sub_codes.find(pSpecificInstrument->InstrumentID) != cur_sub_codes.end())
        {
            cur_sub_codes.erase(pSpecificInstrument->InstrumentID);
        }
    }

    // 如果当前没有订阅的品种, 退登
    if (cur_sub_codes.size() == 0)
    {
        reporter->report("退登 ...");
        logout();
    }
}

//////////////////////////
// 自定义函数
//////////////////////////
void MdSpi::login()
{
    // 参数
    Config *config = Config::getInstance();

    // 登录
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, config->arg(Config::BROKER_ID).data());
    strcpy(req.UserID, config->arg(Config::USER_ID).data());
    strcpy(req.Password, config->arg(Config::PASSWORD).data());
    reporter->report_send_ret(Mdapi->ReqUserLogin(&req, ++requestID));
}

void MdSpi::logout()
{
    // 参数
    Config *config = Config::getInstance();

    // 登录
    CThostFtdcUserLogoutField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, config->arg(Config::BROKER_ID).data());
    strcpy(req.UserID, config->arg(Config::USER_ID).data());
    reporter->report_send_ret(Mdapi->ReqUserLogout(&req, ++requestID));
}

void MdSpi::subdata()
{
    // 等待 all_codes 获取完成
    std::unique_lock<std::mutex> lock(actMutex);
    cv.wait(lock, []
            { return all_codes_done; });

    // 开始订阅数据

    // 参数
    Config *config = Config::getInstance();

    std::vector<std::string> *codes = nullptr;
    if (config->arg(Config::LOCAL_CODES) == "1")
    {
        // 使用本地代码
        codes = &local_codes;
    }
    else
    {
        // 按照交易所订阅
        codes = &(all_codes[config->arg(Config::EXCHANGE)]);
    }

    // 构造数组
    char **sub_code = new char *[codes->size()];
    for (int i = 0; i < codes->size(); i++)
    {
        sub_code[i] = &((*codes)[i])[0];
    }

    // 订阅行情
    reporter->report_send_ret(Mdapi->SubscribeMarketData(sub_code, codes->size()));

    // 释放数组
    delete[] sub_code;
}

void MdSpi::unsubdata()
{
    int code_size = 0;
    while ((code_size = cur_sub_codes.size()) > 0)
    {
        // 构造数组
        char **sub_code = new char *[code_size];
        std::vector<std::string> temp;
        int i = 0;
        for (auto it = cur_sub_codes.begin(); it != cur_sub_codes.end(); ++it)
        {
            temp.push_back(*it);
            sub_code[i] = &(temp[i])[0];
            i++;
        }

        // 订阅行情
        reporter->report(std::string("退订行情: ") + std::to_string(code_size) + " 个");
        reporter->report_send_ret(Mdapi->UnSubscribeMarketData(sub_code, code_size));
        delete[] sub_code;

        Sleep(1500);
    }
}