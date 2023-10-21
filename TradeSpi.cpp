#include "TradeSpi.h"
#include <atomic>

extern std::atomic<int> connect_flag;
extern CThostFtdcTraderApi *Tradeapi;

//////////////////////////
// 回调函数
//////////////////////////
void TradeSpi::OnFrontConnected()
{
    reporter->report("连接建立");
    // reporter->report("登入 ...");
    // login();
}

void TradeSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{

    if (reporter->report_response(pRspInfo))
    {
        reporter->report("登入成功");
        connect_flag = 1;
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
