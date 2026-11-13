/*
 * blackscreen.c  --  极简单文件 Win32 程序，无任何第三方依赖
 *
 * 行为：
 *   - 铺满「工作区」（屏幕去掉任务栏的部分），纯黑，任务栏保持可见
 *   - 普通窗口层级，不置顶（其他窗口可以盖住它，Alt+Tab 可切走）
 *   - 双击窗口 -> 最小化到任务栏（任务栏有按钮，可点回来）
 *   - Esc 或 鼠标右键 -> 退出
 *
 * 编译 MinGW : gcc -O2 -Wall -Wextra -mwindows blackscreen.c -o blackscreen.exe
 * 编译 MSVC  : cl /O2 /W4 blackscreen.c /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib
 */
#include <windows.h>

/* 让窗口刚好盖住工作区（不含任务栏）。bringToFront 只在启动时用一次 */
static void FitToWorkArea(HWND hwnd, BOOL bringToFront)
{
    RECT wa;
    if (IsIconic(hwnd)) return;                       /* 已最小化就别把它弹回来 */
    if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &wa, 0)) return;

    SetWindowPos(hwnd,
        bringToFront ? HWND_NOTOPMOST : NULL,         /* -2：普通窗口最上层，绝非 TOPMOST */
        wa.left, wa.top, wa.right - wa.left, wa.bottom - wa.top,
        SWP_NOACTIVATE | (bringToFront ? 0 : SWP_NOZORDER));
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_ERASEBKGND:
        return 1;                                      /* 全交给 WM_PAINT，避免闪烁 */

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        RECT rc;
        HDC hdc = BeginPaint(hwnd, &ps);
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_LBUTTONDBLCLK:                             /* 双击 -> 最小化到任务栏 */
        ShowWindow(hwnd, SW_MINIMIZE);
        return 0;

    case WM_RBUTTONUP:                                 /* 右键 -> 退出 */
        DestroyWindow(hwnd);
        return 0;

    case WM_KEYDOWN:                                   /* Esc -> 退出 */
        if (wp == VK_ESCAPE) DestroyWindow(hwnd);
        return 0;

    case WM_DISPLAYCHANGE:                             /* 分辨率/任务栏变化后重新贴合 */
    case WM_SETTINGCHANGE:
        FitToWorkArea(hwnd, FALSE);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow)
{
    WNDCLASSW wc = {0};
    HWND hwnd;
    MSG msg;

    (void)hPrev; (void)lpCmd; (void)nShow;

    wc.style         = CS_DBLCLKS;                     /* 关键：没它就收不到双击消息 */
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = L"BlackScreenWnd";
    if (!RegisterClassW(&wc)) return 1;

    hwnd = CreateWindowExW(
        WS_EX_APPWINDOW,                               /* 关键：任务栏有按钮，才能"最小化到任务栏" */
        wc.lpszClassName, L"BlackScreen",
        WS_POPUP | WS_SYSMENU | WS_VISIBLE,            /* 无边框无标题栏；SYSMENU 提供任务栏右键菜单 */
        0, 0, 0, 0, NULL, NULL, hInst, NULL);
    if (!hwnd) return 1;

    FitToWorkArea(hwnd, TRUE);

    while (GetMessageW(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
