#include <windows.h>
#include <stdio.h>
#include <iostream>

int ur, ug, ub;

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    static HDC hdcMem;
    static HBITMAP hbmMem;

    switch (msg)
    {
    case WM_CREATE:
    {
        HDC hdc = GetDC(hwnd);
        hdcMem = CreateCompatibleDC(hdc);
        hbmMem = CreateCompatibleBitmap(hdc, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
        SelectObject(hdcMem, hbmMem);
        ReleaseDC(hwnd, hdc);
        break;
    }
    case WM_CLOSE:
        PostQuitMessage(0);
        break;
    case WM_PAINT:
    {
        // double buffering
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        // draw
        HBRUSH hbr = CreateSolidBrush(RGB(ur, ug, ub));
        FillRect(hdcMem, &rc, hbr);
        DeleteObject(hbr);
        // blit
        BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
    {
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        break;
    }
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    return 0;
}

void CreateConsole()
{
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
}

// keyboard hook
LRESULT KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		KBDLLHOOKSTRUCT* k = (KBDLLHOOKSTRUCT*)lParam;
		if (wParam == WM_KEYDOWN)
		{
			if (k->vkCode == VK_ESCAPE && GetAsyncKeyState(VK_CONTROL) < 0 && GetAsyncKeyState(VK_LWIN) < 0)
			{
				PostQuitMessage(0);
			}
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
	// CreateConsole();
	ur = GetPrivateProfileInt("Color", "R", 0, ".\\TaskbarColor.ini");
	ug = GetPrivateProfileInt("Color", "G", 0, ".\\TaskbarColor.ini");
	ub = GetPrivateProfileInt("Color", "B", 0, ".\\TaskbarColor.ini");
	char buffer[256];
	sprintf(buffer, "%d", (ur ) % 256);
	if (!WritePrivateProfileString("Color", "R", buffer, ".\\TaskbarColor.ini")) {
    std::cout << "Failed to write to INI file!" << std::endl;
}
	
	sprintf(buffer, "%d", (ug ) % 256);
	WritePrivateProfileString("Color", "G", buffer, ".\\TaskbarColor.ini");

	sprintf(buffer, "%d", (ub ) % 256);
	WritePrivateProfileString("Color", "B", buffer, ".\\TaskbarColor.ini");
	std::cout << "Color: " << ur << ", " << ug << ", " << ub << std::endl;

    const char CLASS_NAME[] = "WC_TASKBARCOLOR";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

	int sysWidth = GetSystemMetrics(SM_CXSCREEN);
	int taskbarHeight = GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYFULLSCREEN) - GetSystemMetrics(SM_CYCAPTION);
	int taskbarY = GetSystemMetrics(SM_CYSCREEN) - taskbarHeight;

    HWND hwnd = CreateWindowEx(
        WS_EX_TOOLWINDOW,
        CLASS_NAME,
        "TaskbarColor",
        WS_EX_LAYERED | WS_POPUP,
        0,
		taskbarY, 
		sysWidth,
		taskbarHeight,
        NULL,
		NULL,
		hInst, NULL
    );

    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

	SetForegroundWindow(hwnd);

	// keyboard hook
	HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}