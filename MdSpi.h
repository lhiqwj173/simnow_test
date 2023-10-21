#include "20210406_tradeapi_se_windows/ThostFtdcMdApi.h"
#include "data.h"
#include "reporter.hpp"
#include "config.hpp"

#include <set>

class MdSpi : public CThostFtdcMdSpi
{
public:
    MdSpi(reporter_base *_reporter, data *_data) : reporter(_reporter), data_handles(_data){};

    /// 当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
    virtual void OnFrontConnected() override;

    /// 错误应答
    virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

    /// 当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
    ///@param nReason 错误原因
    ///         0x1001 网络读失败
    ///         0x1002 网络写失败
    ///         0x2001 接收心跳超时
    ///         0x2002 发送心跳失败
    ///         0x2003 收到错误报文
    virtual void OnFrontDisconnected(int nReason) override;

    /// 登录请求响应
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

    /// 登出请求响应
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

    /// 订阅行情应答
    virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

    /// 深度行情通知
    virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) override;

    /// 取消订阅行情应答
    virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

protected:
    // 记录当前订阅的品种
    std::set<std::string> cur_sub_codes;

    // 登入
    void login();

    // 退登
    void logout();

    // 订阅行情
    void subdata();

    // 请求编号
    int requestID = 0;

    // 输出器
    reporter_base *reporter = nullptr;

    // 行情处理器
    data *data_handles = nullptr;

public:
    // 退订行情
    void unsubdata();
};