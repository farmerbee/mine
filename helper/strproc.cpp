#include <cstring>
#include <vector>
#include <iostream>

#include "strproc.h"

static std::vector<char> invalidChars = {
    ' ',
    '\t',
    '\n',
    '[',
    '+',
    '-',
    '#'
};

bool validConfLine(const char* line)
{
    if (strlen(line) <= 1) 
        return false;

    char head = line[0];
    for (auto ch  : invalidChars)
    {
        if (head == ch)
        {
            return false;
        }
    }


    return true;
}

void trimR(char* str, size_t len)
{
    while (len > 0)
    {
        char tail = str[len - 1];
        if (tail == ' ' || tail == '\t' || tail == '\n' || tail == '\r')
        {
            str[len - 1] = '\0';
            len--;
        }
        else
        {
            break;
        }
    }
}

void trimL(char* str, size_t len)
{
    char* validPos = str;
    while (validPos != str + len)
    {
        char curChar = *validPos;
        if (curChar == ' ' || curChar == '\t')
        {
            validPos++;
        }
        else
        {
            break;
        }
    }

    char *newStr = str;
    while (validPos <= str + len)
    {
        *newStr = *validPos;
        newStr++;
        validPos++;
    }
}