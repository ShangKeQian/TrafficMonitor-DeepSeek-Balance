#include <Windows.h>
#include "DeepSeekPlugin.h"

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID)
{
    return TRUE;
}

extern "C" __declspec(dllexport) ITMPlugin* TMPluginGetInstance()
{
    return &CDeepSeekPlugin::Instance();
}
