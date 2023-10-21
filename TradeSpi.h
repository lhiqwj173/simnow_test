#include "20210406_tradeapi_se_windows/ThostFtdcTraderApi.h"
#include "reporter.hpp"
#include "config.hpp"

class TradeSpi : public CThostFtdcTraderSpi
{
public:
    TradeSpi(reporter_base *_reporter) : reporter(_reporter){};

    /// 当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    virtual void OnFrontConnected() override;

    /// 登录请求响应
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

protected:
    // 处理器
    reporter_base *reporter = nullptr;

    // 请求编号
    int requestID = 0;

    // 登入
    void login();
};