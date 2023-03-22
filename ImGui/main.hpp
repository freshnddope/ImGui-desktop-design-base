#pragma once

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "addons/imspinner.h"

#include "font/IconsFontAwesome6Brands.h"
#include "font/IconsFontAwesome6.h"
#include "font/MuseoSansRounded300.h"
#include "font/Museo700.h"
#include "font/trebucbd.h"

#include "texture/background.hpp"

#include <d3d11.h>
#include <D3DX11tex.h>

#pragma comment(lib, "D3DX11.lib")

#include <windows.h>
#include <string> 
#include <stdio.h>
#include <tchar.h>
#include <dwmapi.h>

#include "winver/WinVersion.h"
VersionInfo info;

// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

using namespace std;

// Main window stuff
HWND hwnd;
RECT rc;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Window
{
    const char*     ClassName       = "Class01";
    const char*     WindowName      = "Window Name";
}

namespace Gui
{
	static ImVec2					Size            = { 600 , 300 };
}

void UpdateWindowLocation() 
{
    GetWindowRect(hwnd, &rc);

    if (ImGui::GetWindowPos().x != 0 || ImGui::GetWindowPos().y != 0)
    {
        MoveWindow(hwnd, rc.left + ImGui::GetWindowPos().x, rc.top + ImGui::GetWindowPos().y, Gui::Size.x, Gui::Size.y, TRUE);
        ImGui::SetWindowPos(ImVec2(0.f, 0.f));
    }
}

void RenderBlur(HWND hwnd)
{
    struct ACCENTPOLICY
    {
        int na;
        int nf;
        int nc;
        int nA;
    };
    struct WINCOMPATTRDATA
    {
        int na;
        PVOID pd;
        ULONG ul;
    };

    const HINSTANCE hm = LoadLibrary("user32.dll");
    if (hm)
    {
        typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

        const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hm, "SetWindowCompositionAttribute");
        if (SetWindowCompositionAttribute)
        {
            ACCENTPOLICY policy = { 3, 0, 0, 0 };
            WINCOMPATTRDATA data = { 19, &policy,sizeof(ACCENTPOLICY) };
            SetWindowCompositionAttribute(hwnd, &data);
        }
        FreeLibrary(hm);
    }
}

ImFont* AddCustomDefaultFont(const ImFontConfig* font_cfg_template)
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //ImFont* font = io.Fonts->AddFontFromMemoryCompressedTTF(trebucbd_compressed_data, trebucbd_compressed_size, font_cfg.SizePixels, &font_cfg, glyph_ranges);
    //ImFont* font = io.Fonts->AddFontFromMemoryCompressedTTF(&museo_700_binary, sizeof museo_700_binary, 12, nullptr, glyph_ranges);
    ImFont* font = io.Fonts->AddFontFromMemoryTTF(&museo_700_binary, sizeof museo_700_binary, 14, NULL, io.Fonts->GetGlyphRangesDefault());
    return font;
}