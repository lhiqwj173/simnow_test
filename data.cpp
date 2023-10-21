#include <io.h>
#include <direct.h>
#include <fstream>
#include "data.h"

file_keeper *file_keeper::getInstance()
{
    static file_keeper instance;
    return &instance;
}
std::string file_keeper::name() { return "file_keeper"; }
void file_keeper::handle(const CThostFtdcDepthMarketDataField *data)
{
    // 保存到本地
    std::ofstream *&file = _files[data->InstrumentID];
    if (!file || (!file->is_open()))
    {
        // 检查路径
        if (_access("raw", 0) == -1)
        {
            _mkdir("raw");
        }

        std::string date_folder = std::string("raw//") + data->TradingDay;
        if (_access(date_folder.data(), 0) == -1)
        {
            _mkdir(date_folder.data());
        }

        // 打开文件 raw/date/code.csv
        std::string file_name = date_folder + "//" + data->InstrumentID + ".csv";
        file = new std::ofstream(file_name.data(), std::ios::app);
    }

    // 写入文件
    *file << data->TradingDay << ","
          << data->ExchangeID << ","
          << data->ExchangeInstID << ","
          << data->InstrumentID << ","
          << data->LastPrice << ","
          << data->PreSettlementPrice << ","
          << data->PreClosePrice << ","
          << data->PreOpenInterest << ","
          << data->OpenPrice << ","
          << data->HighestPrice << ","
          << data->LowestPrice << ","
          << data->Volume << ","
          << data->Turnover << ","
          << data->OpenInterest << ","
          << data->ClosePrice << ","
          << data->SettlementPrice << ","
          << data->UpperLimitPrice << ","
          << data->LowerLimitPrice << ","
          << data->PreDelta << ","
          << data->CurrDelta << ","
          << data->UpdateTime << ","
          << data->UpdateMillisec << ","
          << data->BidPrice1 << ","
          << data->BidVolume1 << ","
          << data->AskPrice1 << ","
          << data->AskVolume1 << ","
          << data->BidPrice2 << ","
          << data->BidVolume2 << ","
          << data->AskPrice2 << ","
          << data->AskVolume2 << ","
          << data->BidPrice3 << ","
          << data->BidVolume3 << ","
          << data->AskPrice3 << ","
          << data->AskVolume3 << ","
          << data->BidPrice4 << ","
          << data->BidVolume4 << ","
          << data->AskPrice4 << ","
          << data->AskVolume4 << ","
          << data->BidPrice5 << ","
          << data->BidVolume5 << ","
          << data->AskPrice5 << ","
          << data->AskVolume5 << ","
          << data->AveragePrice << ","
          << data->ActionDay << std::endl;
}
file_keeper::~file_keeper()
{
    for (auto &file : _files)
    {
        if (file.second)
        {
            if (file.second->is_open())
            {
                file.second->close();
            }

            delete file.second;
        }
    }
}

mem_keeper *mem_keeper::getInstance()
{
    static mem_keeper instance;
    return &instance;
}
std::string mem_keeper::name() { return "mem_keeper"; }
void mem_keeper::handle(const CThostFtdcDepthMarketDataField *data)
{
    std::vector<CThostFtdcDepthMarketDataField> &__data = _datas[data->InstrumentID];

    __data.push_back(*data);
    CThostFtdcDepthMarketDataField &new_data = __data[__data.size() - 1];

    // 字符字段拷贝
    memset(new_data.TradingDay, 0, sizeof(new_data.TradingDay));
    strcpy(new_data.TradingDay, data->TradingDay);

    memset(new_data.ExchangeID, 0, sizeof(new_data.ExchangeID));
    strcpy(new_data.ExchangeID, data->ExchangeID);

    memset(new_data.UpdateTime, 0, sizeof(new_data.UpdateTime));
    strcpy(new_data.UpdateTime, data->UpdateTime);

    memset(new_data.ActionDay, 0, sizeof(new_data.ActionDay));
    strcpy(new_data.ActionDay, data->ActionDay);

    memset(new_data.InstrumentID, 0, sizeof(new_data.InstrumentID));
    strcpy(new_data.InstrumentID, data->InstrumentID);

    memset(new_data.ExchangeInstID, 0, sizeof(new_data.ExchangeInstID));
    strcpy(new_data.ExchangeInstID, data->ExchangeInstID);
}

// 更新数据
void data::update(const CThostFtdcDepthMarketDataField *data)
{
    for (auto &handle : handles)
    {
        reporter.report(std::string("处理: ") + handle.first);
        handle.second->handle(data);
    }
}

// 增加处理器
void data::add_handler(handler_base *handler)
{
    if (handles.find(handler->name()) == handles.end())
    {
        reporter.report(std::string("添加处理器: ") + handler->name());
        handles[handler->name()] = handler;
    }
}

// 删除处理器
void data::remove_handler(std::string handle_name)
{
    if (handles.find(handle_name) != handles.end())
    {
        reporter.report(std::string("删除处理器: ") + handle_name);
        handles.erase(handle_name);
    }
}
