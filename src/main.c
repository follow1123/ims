#include <windows.h>
#include <minwindef.h>
#include <winnt.h>
#include <errhandlingapi.h>

#include <stdio.h>
#include <stdlib.h>

/* 处理位图回调方法 */
typedef int (*HandleBitMap)(BITMAP, BYTE*);

/* 切换输入法模式 */
BOOL switch_input_mode(HWND hwnd) {
    return PostMessageW(hwnd, WM_LBUTTONDOWN, 0, 0) &&
        PostMessageW(hwnd, WM_LBUTTONUP, 0, 0);
}

/* 获取系统托盘处输入指示器句柄 */
HWND get_ime_handle() {
    HWND system_tray = FindWindowW(L"Shell_TrayWnd", NULL);
    if (system_tray == NULL) return NULL;
    HWND tray_notify = FindWindowExW(system_tray, NULL, L"TrayNotifyWnd", NULL);
    if (tray_notify == NULL) return NULL;
    HWND input_indicator = FindWindowExW(tray_notify, NULL, L"TrayInputIndicatorWClass", NULL);
    if (input_indicator == NULL) return NULL;
    return FindWindowExW(input_indicator, NULL, L"IMEModeButton", NULL);
}

/* 处理位图 */
int handle_bitmap(BITMAP bitmap, BYTE* pixels) {
    // 从右下角开始扫描
    int sum;
    int flag = 0;
    int yLimit = 0;
    for (int y = bitmap.bmHeight - 1; y >= yLimit; y--) {
        for (int x = bitmap.bmWidth - 1; x >= 0; x--) {
            // 32位位图
            int offset = y * bitmap.bmWidthBytes + x * 4;
            int bitSum = pixels[offset] + pixels[offset + 1] + pixels[offset + 2];
            if (bitSum != 0) {
                if (yLimit == 0) {
                    yLimit = y;
                    flag = x;
                } else {
                    if (flag == x + 1) {
                        flag = x;
                    }else {
                        return 1;
                    }
                }
            }
            sum += bitSum;
        }
    }
    return 2;
}

/* 获取当前输入模式 */
BOOL get_input_mode(HWND hwnd, HandleBitMap callback, int *mode) {
    BOOL b = 1;
    // 从窗口中获取位图
    HDC hdcWindow = GetDC(hwnd);
    if (hdcWindow == NULL) return 0;
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    if (hdcMem == NULL) return 0;
    RECT rect;
    b = GetClientRect(hwnd, &rect);
    if (b == FALSE) return b;
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, width, height);
    if (hBitmap == NULL) return 0;
    HGDIOBJ hGdiObj = SelectObject(hdcMem, hBitmap);
    if (hGdiObj == NULL) return 0;

    // 打印窗口内容到内存DC
    b = PrintWindow(hwnd, hdcMem, PW_CLIENTONLY);
    if (b == FALSE) return b;
    BITMAP bmp;
    b = GetObjectW(hBitmap, sizeof(BITMAP), &bmp);
    if (b == FALSE) return b;

    // 获取位图的数据
    BYTE* pPixels = (BYTE*)GlobalAlloc(GPTR, bmp.bmWidthBytes * bmp.bmHeight);
    if (pPixels == NULL) return 0;
    LONG l = GetBitmapBits(hBitmap, bmp.bmWidthBytes * bmp.bmHeight, pPixels);
    if (l == FALSE) return 0;

    // 判断输入模式
    *mode = callback(bmp, pPixels);

    // 释放资源
    GlobalFree(pPixels);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);
    return b;
}

int main(int argc, char *argv[]) {
    // 获取输入指示器的窗口句柄
    HWND hwnd = get_ime_handle();
    if (hwnd == NULL) {
        fprintf(stderr, "get ime handle error\n");
        return 1;
    }
    int mode;
    BOOL b = get_input_mode(hwnd, handle_bitmap, &mode);
    if (b == FALSE) {
        fprintf(stderr, "get input mode error code: %lu\n", GetLastError());
        return 1;
    }
    // 获取第一个参数
    char *param = argv[1];
    if (param == NULL) {
        printf("%d\n", mode);
        return 0;
    }
    // 转换为数字
    int m = atoi(param);
    if (m == mode) return 0;
    b = switch_input_mode(hwnd);
    if (b == FALSE) {
        fprintf(stderr, "switch input mode error code: %lu\n", GetLastError());
        return 1;
    }
    return 0;
}

