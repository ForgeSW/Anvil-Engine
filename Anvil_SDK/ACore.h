#pragma once
// ACore.h
#ifdef ANVIL_SDK
#define ANVIL_API __declspec(dllexport)
#else
#define ANVIL_API __declspec(dllimport)
#endif
