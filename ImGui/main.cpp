//============================================== ImGui Desktop ==============================================//
//
//   Dear ImGui: standalone example application for DirectX 11 - Desktop
//   If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
//   Read online: https://github.com/ocornut/imgui/tree/master/docs
// 
//===========================================================================================================//

#include "main.hpp"

// Main code
int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    WNDCLASSEX windowClass = { };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_CLASSDC;
    windowClass.lpfnWndProc = WndProc;
    windowClass.cbClsExtra = NULL;
    windowClass.cbWndExtra = NULL;
    windowClass.hInstance = GetModuleHandleA(0); // nullptr
    windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
    windowClass.hCursor = LoadCursor(0, IDC_ARROW);
    windowClass.hbrBackground = nullptr;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = Window::ClassName;
    windowClass.hIconSm = LoadIcon(0, IDI_APPLICATION);

    RegisterClassEx(&windowClass);
    hwnd = CreateWindowEx(NULL, windowClass.lpszClassName, Window::WindowName, WS_POPUP | CW_USEDEFAULT, (GetSystemMetrics(SM_CXSCREEN) / 2) - (Gui::Size.x / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (Gui::Size.y / 2), Gui::Size.x, Gui::Size.y, 0, 0, 0, 0);

    SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    POINT mouse;
    rc = { 0 };
    GetWindowRect(hwnd, &rc);

    //RenderBlur(hwnd);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Getting some shutdown stuff
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.IniFilename = NULL; // Remove the imgui.ini

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 5;

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    AddCustomDefaultFont(nullptr);

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    static const ImWchar icons_ranges_brands[] = { ICON_MIN_FAB, ICON_MAX_16_FAB, 0 };
    ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
    ImFont* FontAwesome = io.Fonts->AddFontFromMemoryCompressedTTF(fa6_solid_compressed_data, fa6_solid_compressed_size, 14.f, &icons_config, icons_ranges);
    ImFont* FontAwesomeBrands = io.Fonts->AddFontFromMemoryCompressedTTF(fa_brands_400_compressed_data, fa_brands_400_compressed_size, 14.f, &icons_config, icons_ranges_brands);

    // Main loop
    bool loaderOpen = true;
    if (WinVersion::GetVersion(info) && info.Major <= 6)
    {
        MessageBox(hwnd, "your operating system is not supported", "Error", MB_ICONERROR);
    }
    else while ( loaderOpen && (info.Major > 6) )
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }
        }
        if (!loaderOpen)
        {
            break;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            ImGuiWindowFlags main_window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
            ImGui::SetNextWindowPos({0 , 0}, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(Gui::Size);
            ImGui::Begin(Window::WindowName, &loaderOpen, main_window_flags);
            UpdateWindowLocation();
            {
                ImGui::Text("Hello world !");
            }
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
       
        const float clear_color_with_alpha[4] = { 0 };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Never set the 1 to 0 or the panel movment will get fucked
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}