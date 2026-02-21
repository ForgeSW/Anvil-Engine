#pragma once
// AShader.h
#include "ACore.h"
#include <sstream>
#include <string_view>

class ANVIL_API AShader
{
  public:
    AShader(int vertResID, int fragResID);
    AShader(std::string_view vPath, std::string_view fPath);
    ~AShader();
    void     CheckCompileErrors(uint32_t shader, std::string type);
    void     Use();
    uint32_t GetID() const
    {
        return m_ID;
    }

  private:
    uint32_t    m_ID;
    void        Compile(const char* vCode, const char* fCode);
    std::string LoadFromResource(int resID);
};