//#define //_CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <dwmapi.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <io.h>              // Для _setmode
#include <fcntl.h>           // Для _O_U16TEXT
#include <locale>            // Для std::locale

#pragma comment(lib, "dwmapi.lib")

void MonitorWindows();

// Функция для открытия консоли с поддержкой UTF-8
//void CreateConsole() {
    //if (AllocConsole()) {
        //freopen("CONOUT$", "w", stdout);
        //freopen("CONIN$", "r", stdin);

        //SetConsoleOutputCP(CP_UTF8);
        //SetConsoleCP(CP_UTF8);

        //std::wcout.imbue(std::locale("en_US.UTF-8"));
        //std::wcout << L"Консоль успешно открыта." << std::endl;
    //}
    //else {
        //std::wcerr << L"Не удалось создать консоль." << std::endl;
    //}
//}

//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    //CreateConsole();
    //std::wcout << L"Привет из консоли!" << std::endl;

    //std::thread monitorThread(MonitorWindows);

    //std::wcout << L"Окна мониторятся. Нажмите любую клавишу для завершения." << std::endl;
    //std::cin.get();

    //return 0;
//}

enum ACCENT_STATE {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_ENABLE_HOSTBACKDROP = 5,
    ACCENT_ENABLE_ACRYLICHOSTBACKDROP = 6,
    ACCENT_ENABLE_BLUR = 7,
};

struct ACCENT_POLICY {
    ACCENT_STATE nAccentState;
    int nFlags;
    int nColor;
    int nAnimationId;
};

struct WINCOMPATTRDATA {
    int nAttribute;
    PVOID pData;
    ULONG ulDataSize;
};

// Объявляем функцию SetWindowCompositionAttribute
using SetWindowCompositionAttributeFunc = BOOL(WINAPI*)(HWND, WINCOMPATTRDATA*);

void EnableBlur(HWND hWnd) {
    HMODULE hModule = GetModuleHandle(L"user32.dll");
    if (hModule) {
        auto SetWindowCompositionAttribute =
            reinterpret_cast<SetWindowCompositionAttributeFunc>(
                GetProcAddress(hModule, "SetWindowCompositionAttribute"));

        if (SetWindowCompositionAttribute) {
            // Получаем акцентный цвет из системы
            COLORREF accentColor;
            BOOL isOpaqueBlend;
            HRESULT hr = DwmGetColorizationColor(&accentColor, &isOpaqueBlend);

            //if (FAILED(hr)) {
                //std::wcout << L"Не удалось получить акцентный цвет, HRESULT: " << hr << std::endl;
                //return;
            //}

            // Преобразуем COLORREF в ARGB формат
            int alpha = 200; // Прозрачность 200 из 255
            int red = GetRValue(accentColor);
            int green = GetGValue(accentColor);
            int blue = GetBValue(accentColor);
            int color = (alpha << 24) | (red << 16) | (green << 8) | blue;

            ACCENT_POLICY policy = { ACCENT_ENABLE_BLURBEHIND, 0, color, 0 };
            WINCOMPATTRDATA data = { 19, &policy, sizeof(policy) };

            SetWindowCompositionAttribute(hWnd, &data);
        }
        else {
            //std::wcout << L"Не удалось получить SetWindowCompositionAttribute" << std::endl;
        }
    }
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam) {
    if (IsWindowVisible(hWnd)) {
        wchar_t className[256];
        GetClassName(hWnd, className, 256);

        wchar_t windowTitle[256];
        GetWindowText(hWnd, windowTitle, 256);

        if (wcscmp(className, L"Notepad") == 0 ||
            wcscmp(className, L"CabinetWClass") == 0) {
            //std::wcout << L"Найдено окно: " << className << L" (HWND: " << hWnd << L")" << std::endl;
            EnableBlur(hWnd);
        }
    }
    return TRUE;
}

void MonitorWindows() {
    while (true) {
        EnumWindows(EnumWindowsProc, 0);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Точка входа в графическое приложение
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    std::thread monitorThread(MonitorWindows);
    monitorThread.detach(); // Отсоединяем поток для работы в фоне

    // Основной цикл программы
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
