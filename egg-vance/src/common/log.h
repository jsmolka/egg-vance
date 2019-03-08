#pragma once

#include <iostream>

#define log() Log(__FUNCTION__)

class Log
{
public:
    Log(const char* function) 
    { 
        std::cout << function << " - "; 
    };
    
    ~Log() 
    { 
        std::cout << "\n"; 
    };

    template<typename T>
    Log& operator<<(const T& value)
    {
        std::cout << value;

        return *this;
    }
};