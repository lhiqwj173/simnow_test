#include "20210406_tradeapi_se_windows/ThostFtdcMdApi.h"
#include "MdSpi.h"
#include "TradeSpi.h"
#include "handles.h"
#include "reporter.hpp"
#include "config.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <mutex>
#include <fstream>
#include <atomic>

// 全局变量
// api
CThostFtdcMdApi *Mdapi;
CThostFtdcTraderApi *Tradeapi;

// 连接标志
std::atomic<int> connect_flag;

// 可订阅代码 <交易所id: <代码id>>
std::map<std::string, std::vector<std::string>> all_codes;

// 本地读取代码
std::vector<std::string> local_codes;

// api线程互斥量
std::mutex actMutex;
std::condition_variable cv;
bool all_codes_done = false;

// 本地读取
void read_codes(std::vector<std::string> &codes)
{
    std::ifstream infile("codes.txt");
    std::string line;
    while (std::getline(infile, line))
    {
        codes.push_back(line);
    }
}

int main()
{
    // 参数
    // char front_addr[] = "tcp://180.168.146.187:10010";
    // std::string broker_id;
    // std::string user_id;
    // std::string password;
    // printf("经纪商代码：");
    // std::cin >> broker_id;
    // printf("账号：");
    // std::cin >> user_id;
    // printf("密码：");
    // std::cin >> password;
    // 读取配置文件
    Config *config = Config::getInstance();

    // 本地读取代码
    read_codes(local_codes);
    connect_flag = 0;
    ///////////////
    // 行情
    ///////////////
    Mdapi = CThostFtdcMdApi::CreateFtdcMdApi(); // 实例化MdApi
    reporter_md _md{};                          // 处理类

    // 数据处理管理类
    handles data_handles;
    data_handles.add_handler(file_keeper::getInstance()); // 添加文件储存
    data_handles.add_handler(mem_keeper::getInstance());  // 添加内存储存

    MdSpi mdCollector{&_md, &data_handles};                         // 响应类
    Mdapi->RegisterSpi(&mdCollector);                               // 注册响应类
    Mdapi->RegisterFront(&(config->arg(Config::MD_FRONT_ADDR))[0]); // 注册前置地址
    Mdapi->Init();                                                  // 初始化

    ///////////////
    // 交易
    // ErrorID=3, ErrorMsg=CTP:不合法的登录
    // 无法登入 拿不到标的列表
    // 先从 code.txt 输入
    ///////////////
    Tradeapi = CThostFtdcTraderApi::CreateFtdcTraderApi();                // 实例化TraderApi
    reporter_trade _trade{};                                              // 处理类
    TradeSpi trader{&_trade};                                             // 响应类
    Tradeapi->RegisterSpi(&trader);                                       // 注册响应类
    Tradeapi->RegisterFront(&(config->arg(Config::TRADE_FRONT_ADDR))[0]); // 注册前置地址
    Tradeapi->Init();                                                     // 初始化

    Sleep(3000);
    if (connect_flag == 0)
    {
        std::cout << "未连接异常 退出" << std::endl;
        return 0;
    }

    // 等待 60 秒
    for (int i = 0; i < 10; i++)
    {
        Sleep(1000);
    }

    // 准备退出
    // 退订
    mdCollector.unsubdata();
    trader.logout();

    // 等待线程结束
    // Mdapi->Join();
    // Tradeapi->Join();

    // 等待退出
    while (connect_flag > 0)
    {
        Sleep(1000);
    }

    return 0;
}
