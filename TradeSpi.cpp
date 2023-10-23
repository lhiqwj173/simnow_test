#include "TradeSpi.h"
#include <atomic>
#include <map>
#include <vector>
#include <string>

extern std::atomic<int> connect_flag;
extern CThostFtdcTraderApi *Tradeapi;
extern std::map<std::string, std::vector<std::string>> all_codes;

// api线程互斥量
extern std::mutex actMutex;
extern std::condition_variable cv;
extern bool all_codes_done;

//////////////////////////
// 回调函数
//////////////////////////
void TradeSpi::OnFrontConnected()
{
    reporter->report("连接建立");
    // Tradeapi->Release();
    reporter->report("登入 ...");
    login();
}

void TradeSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (reporter->report_response(pRspInfo))
    {
        reporter->report("登入成功");
        connect_flag += 1;

        // 查询codes
        ask_codes();
    }
}

void TradeSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (reporter->report_response(pRspInfo))
    {
        reporter->report("退登成功");
        connect_flag -= 1;
        Tradeapi->Release();
    }
}

void TradeSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    // pRspInfo 不可读

    std::vector<std::string> &_exchange_codes = all_codes[pInstrument->ExchangeID];
    _exchange_codes.push_back(pInstrument->InstrumentID);

    if (bIsLast == true)
    {
        reporter->report("查询代码完成");
        std::lock_guard<std::mutex> lock(actMutex);
        all_codes_done = true;
        cv.notify_one();
    }
}

//////////////////////////
// 自定义函数
//////////////////////////
void TradeSpi::login()
{
    // 参数
    Config *config = Config::getInstance();

    // 登录
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, config->arg(Config::BROKER_ID).data());
    strcpy(req.UserID, config->arg(Config::USER_ID).data());
    // strcpy(req.UserID, "218981");
    strcpy(req.Password, config->arg(Config::PASSWORD).data());
    reporter->report_send_ret(Tradeapi->ReqUserLogin(&req, ++requestID));
}

void TradeSpi::logout()
{
    // 参数
    Config *config = Config::getInstance();

    // 登录
    CThostFtdcUserLogoutField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, config->arg(Config::BROKER_ID).data());
    strcpy(req.UserID, config->arg(Config::USER_ID).data());
    reporter->report_send_ret(Tradeapi->ReqUserLogout(&req, ++requestID));
}

void TradeSpi::ask_codes()
{
    reporter->report("查询代码可交易标的");

    // 清空
    all_codes.clear();

    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(req));
    reporter->report_send_ret(Tradeapi->ReqQryInstrument(&req, ++requestID));
}