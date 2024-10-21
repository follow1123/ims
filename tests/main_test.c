#include <windows.h>
#include <synchapi.h>
#include <minwindef.h>
#include <winnt.h>
#include <errhandlingapi.h>

#include <stdio.h>
#include <stdlib.h>

#define RED "\033[31m"      // 红色
#define YELLOW "\033[33m"   // 黄色
#define GREEN "\033[32m"    // 绿色
#define RESET "\033[0m"     // 重置颜色

/* 处理位图回调方法 */
typedef int (*HandleBitMap)(BITMAP, BYTE*, int);

typedef int (*TestMethod)();

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
int t_handle_bitmap(BITMAP bitmap, BYTE* pixels, int print) {
    // 从右下角开始扫描
    int sum;
    int mode = 2;
    int flag = 0;
    int yLimit = 0;
    for (int y = bitmap.bmHeight - 1; y >= 0; y--) {
        for (int x = bitmap.bmWidth - 1; x >= 0; x--) {
            // 32位位图
            int offset = y * bitmap.bmWidthBytes + x * 4;
            int bitSum = pixels[offset] + pixels[offset + 1] + pixels[offset + 2];
            if (print) printf("%s ", (bitSum == 0 ? "." : "#"));
            if (bitSum != 0 && mode == 2 && y >= yLimit) {
                if (yLimit == 0) {
                    yLimit = y;
                    flag = x;
                } else {
                    if (flag == x + 1) {
                        flag = x;
                    }else {
                        mode = 1;
                    }
                }
            }
            sum += bitSum;
        }
        if (print) printf("\n");
    }
    return mode;
}

/* 获取当前输入模式 */
BOOL get_input_mode(HWND hwnd, HandleBitMap callback, int *mode, int print) {
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
    *mode = callback(bmp, pPixels, print);

    // 释放资源
    GlobalFree(pPixels);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcWindow);
    return b;
}

int test_input_mode_is_correct() {
    BOOL b = 1;
    HWND hwnd = get_ime_handle();
    if (hwnd == NULL) return 0;
    int mode;
    b = get_input_mode(hwnd, t_handle_bitmap, &mode, 1);
    if (b == FALSE) return b;

    printf("%s:%d\n", (mode == 1 ? "english" : "chinese"), mode);
    return mode == 1 || mode == 2;
}

int test_input_mode_is_switch() {
    BOOL b = 1;
    HWND hwnd = get_ime_handle();
    if (hwnd == NULL) return 0;
    int mode;
    b = get_input_mode(hwnd, t_handle_bitmap, &mode, 0);
    if (b == FALSE) return b;
    b = switch_input_mode(hwnd);
    if (b == FALSE) return b;

    Sleep(100);
    int m;
    b = get_input_mode(hwnd, t_handle_bitmap, &m, 0);
    if (b == FALSE) return b;
    return m != mode;
}

void test_wrapper(char *desc, TestMethod tm) {
    printf("start test %s:\n", desc);
    int res = tm();
    printf("test %s\n", (res == 0 ? RED "failed" RESET: GREEN "passed" RESET));
}

int main(int argc, char *argv[]) {
    test_wrapper("input mode is correct", test_input_mode_is_correct);
    test_wrapper("input mode is switch", test_input_mode_is_switch);
    return 0;
}

