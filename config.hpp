#pragma once
#include <string>
#include <map>
#include <Windows.h>
#include <iostream>

// 参数相关
class Config
{
private:
    // 私有构造函数，防止在类外部被实例化
    Config()
    {
        read_args();
    }
    Config(const Config &) = delete;            // 禁止拷贝构造函数
    Config &operator=(const Config &) = delete; // 禁止赋值运算符

public:
    static Config *getInstance()
    {
        static Config instance;
        return &instance;
    }

public:
    // 参数枚举类型
    // front_addr
    // broker_id
    // user_id
    // password
    enum ARGS
    {
        MD_FRONT_ADDR,
        TRADE_FRONT_ADDR,
        BROKER_ID,
        USER_ID,
        PASSWORD,
        LOCAL_CODES,
        EXCHANGE,
    };

private:
    // 储存参数名称
    std::map<ARGS, std::string> ARGS_NAME = {
        {MD_FRONT_ADDR, "md_front_addr"},
        {TRADE_FRONT_ADDR, "trade_front_addr"},
        {BROKER_ID, "broker_id"},
        {USER_ID, "user_id"},
        {PASSWORD, "password"},
        {LOCAL_CODES, "if_local_codes"},
        {EXCHANGE, "exchangeid"},
    };

    // 储存原始字符串参数
    std::map<ARGS, std::string> args;

    // 读取配置文件参数
    // 文件格式: 参数名称=参数值
    void read_args()
    {
        // 配置文件路径, 优先使用当前目录下的config.txt
        char config_path[256];
        GetCurrentDirectoryA(256, config_path);
        strcat(config_path, "\\config.txt");

        // 打开文件
        FILE *fp = fopen(config_path, "r");

        // 读取文件
        if (fp)
        {
            char line[256];
            while (fgets(line, 256, fp))
            {
                // 去除换行符
                char *p = strchr(line, '\n');
                if (p)
                {
                    *p = '\0';
                }

                // 判断是否是注释
                if (line[0] == '#')
                {
                    continue;
                }

                // 判断是否是空行
                if (strlen(line) == 0)
                {
                    continue;
                }

                // 判断是否是参数
                char *p2 = strchr(line, '=');
                if (p2)
                {
                    // 参数名称
                    char name[256];
                    strncpy(name, line, p2 - line);
                    name[p2 - line] = '\0';

                    // 参数值
                    char value[256];
                    strcpy(value, p2 + 1);

                    // 储存参数
                    for (auto it = ARGS_NAME.begin(); it != ARGS_NAME.end(); it++)
                    {
                        if (strcmp(it->second.c_str(), name) == 0)
                        {
                            std::cout << it->second << ": " << value << std::endl;
                            args[it->first] = value;
                            break;
                        }
                    }
                }
            }

            fclose(fp);
        }
        else
        {
            printf("配置文件不存在\n");
        }
    }

public:
    // 获取参数
    std::string arg(ARGS arg)
    {
        return args[arg];
    }
};
