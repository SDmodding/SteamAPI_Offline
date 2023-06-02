#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <d3d11.h>
#include "../../SDK/SDK/SDK/_Includes.hpp"

// https://github.com/brofield/simpleini
#include <SimpleIni.h>
static CSimpleIniA g_Configuration;

// Init Functions
void InitializeFakeSteamInfo()
{
    const char* m_Name = g_Configuration.GetValue("Configuration", "Name", nullptr);
    if (m_Name && strlen(m_Name) > 0)
    {
        UFG::qString* m_OnlineName = reinterpret_cast<UFG::qString*>(UFG_RVA(0x249D978));
        m_OnlineName->Set(m_Name);
    }
}

// Exports
extern "C" __declspec(dllexport) bool __fastcall SteamAPI_Init()
{
    return false;
}

extern "C" __declspec(dllexport) bool __fastcall SteamAPI_IsSteamRunning()
{
    static bool m_Once = true;
    if (m_Once)
    {
        m_Once = false;
        InitializeFakeSteamInfo();
    }

    return false;
}

extern "C" __declspec(dllexport) void __fastcall SteamAPI_RegisterCallResult(void* m_Callback, void* m_APICall)
{
    return;
}

extern "C" __declspec(dllexport) void __fastcall SteamAPI_RegisterCallback(void* m_Callback, int m_CallbackID)
{
    return;
}   

extern "C" __declspec(dllexport) bool __fastcall SteamAPI_RestartAppIfNecessary(uint32_t m_AppID)
{
    return false;
}

extern "C" __declspec(dllexport) void __fastcall SteamAPI_RunCallbacks()
{
    return;
}

extern "C" __declspec(dllexport) void __fastcall SteamAPI_SetMiniDumpComment(const char* m_Msg)
{
    return;
}

extern "C" __declspec(dllexport) void __fastcall SteamAPI_WriteMiniDump(uint32_t m_StructuredExceptionCode, void* m_ExceptionInfo, uint32_t m_BuildID)
{
    return;
}

extern "C" __declspec(dllexport) void __fastcall SteamAPI_Shutdown()
{
    return;
}

extern "C" __declspec(dllexport) void __fastcall SteamAPI_UnregisterCallResult(void* m_Callback, int m_CallbackID)
{
    return;
}

extern "C" __declspec(dllexport) void __fastcall SteamAPI_UnregisterCallback(void* m_Callback)
{
    return;
}

extern "C" __declspec(dllexport) void* __fastcall SteamApps()
{
    return nullptr;
}

extern "C" __declspec(dllexport) void* __fastcall SteamClient()
{
    return nullptr;
}

extern "C" __declspec(dllexport) void* __fastcall SteamFriends()
{
    return nullptr;
}

extern "C" __declspec(dllexport) void* __fastcall SteamUser()
{
    return nullptr;
}

extern "C" __declspec(dllexport) void* __fastcall SteamUserStats()
{
    return nullptr;
}

extern "C" __declspec(dllexport) void* __fastcall SteamUtils()
{
    return nullptr;
}

// Functions
namespace Memory
{
    bool Patch(uintptr_t m_Address, uint8_t* m_Bytes, size_t m_Size)
    {
        DWORD m_OldProtection = 0x0;
        if (!VirtualProtect(reinterpret_cast<void*>(m_Address), m_Size, PAGE_EXECUTE_READWRITE, &m_OldProtection))
            return false;

        memcpy(reinterpret_cast<void*>(m_Address), m_Bytes, m_Size);
        VirtualProtect(reinterpret_cast<void*>(m_Address), m_Size, m_OldProtection, &m_OldProtection);
        return true;
    }
}

int __stdcall DllMain(HMODULE m_Module, DWORD m_CallReason, void* m_Reserved)
{
    if (m_CallReason == DLL_PROCESS_ATTACH)
    {
        if (g_Configuration.LoadFile("steam_offline_config.ini") != SI_OK)
            MessageBoxA(0, "Couldn't load file: 'steam_offline_config.ini'.", "SteamAPI_Offline", MB_OK | MB_USERICON | MB_TOPMOST | MB_SETFOREGROUND);

        // SteamIntegration::InitAPI
        {
            uint8_t m_Bytes[] = { 0xB0, 0x01, 0xC3 };
            Memory::Patch(UFG_RVA(0x427160), m_Bytes, sizeof(m_Bytes));
        }

        // UFG::UIGfxTranslator::getSystemLanguage
        {
            uint8_t m_LanguageID = max(4, min(static_cast<uint8_t>(g_Configuration.GetLongValue("Configuration", "Language", 7)), 23));
            uint8_t m_Bytes[] = { 0xB8, m_LanguageID, 0x00, 0x00, 0x00, 0xC3 };
            Memory::Patch(UFG_RVA(0xA2BBE0), m_Bytes, sizeof(m_Bytes));
        }

        // UFG::UIHKScreenSocialHub::PopulateOnlineStatusIcon
        {
            uint8_t m_Bytes[] = { 0xC3 };
            Memory::Patch(UFG_RVA(0x5F9D40), m_Bytes, sizeof(m_Bytes));
        }

        // HasBeenWarned (COMMON_TRC_PC_CONNECTION_FAILED)
        *reinterpret_cast<bool*>(UFG_RVA(0x24337C8)) = true;
    }

    return 1;
}
