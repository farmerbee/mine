#include <iostream>
#include <fstream>
#include <cstring>

#include "config.h"
#include "strproc.h"

using namespace mine;

#define BUFFSIZE 502

Config* Config::m_instance = nullptr;

Config::Config()
    : m_items()
{
}

Config::~Config()
{

}

Config* Config::getInstance()
{
    if (!m_instance)
    {
        m_instance = new Config();
    }

    return m_instance;
}

std::string Config::getValue(std::string key) 
{
    std::string value = "";
    if (m_items.count(key))
    {
        value = m_items[key];
    }

    return value; 
}

bool Config::load(const std::string& path)
{
    char buff[BUFFSIZE];

    if (path.empty())
        return false;
    
    std::ifstream  fs(path, std::fstream::in);
    if (fs.fail())
        return false;
    
    while (!fs.eof())
    {
        memset(buff, 0, sizeof(buff));    
        fs.getline(buff, BUFFSIZE -1);
        if (!validConfLine(buff))
        {
            continue;
        }

        char* equalPos = strchr(buff, strlen(buff));
        if (!equalPos)
        {
            continue;
        }
        char key[BUFFSIZE], value[BUFFSIZE];
        strncpy(key, buff, static_cast<size_t>(equalPos - buff));
        strcpy(value, equalPos + 1);

        trimR(key, strlen(key));
        trimL(key, strlen(key));
        trimR(value, strlen(value));
        trimL(value, strlen(value));

        m_items[key] = value;
    }

    fs.close();

    return true;
}