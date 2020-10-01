#include <iostream>
#include <windows.h>

/* 1. Изучение событийной архитектору Windows-приложений, механизма обработки сообщений, механизма перерисовки окна.

- Разработать программу, позволяющую передвигать с помощью клавиатуры и мыши спрайт (окрашенный прямоугольник или эллипс) внутри рабочей области окна.
- Обеспечить работу колесика мыши. Прокручивание двигает спрайт по вертикали. С удерживаемой клавишей Shift прокручивание колесика двигает спрайт по горизонтали.
- Заменить спрайт на картинку с непрямоугольным контуром.
- Придать спрайту движение с отскоком от границ окна.

*/

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void ShowBitmap(HWND hWnd, int &offsetX, int &offsetY);

void CorrectOffset(int &offsetX, int &offsetY, int clientWidth, int clientHeight);

int wheelDelta = 0;
int offsetX = 0;
int offsetY = 0;
const int DELTA = 20;
const int DELTA_PUSH = 10;
const int SCALE = 5;
const CHAR *const BMP_PATH = "test.bmp";
HBITMAP hBitmap;
bool isLeftButtonDown = false;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR lpCmdLine,
                     int nCmdShow) {
    WNDCLASSEX wcex;
    HWND hWnd;
    MSG msg;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "SomeWindowClass";
    wcex.hIconSm = wcex.hIcon;

    RegisterClassEx(&wcex);


    hWnd = CreateWindow("SomeWindowClass", "Some Window",
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0,
                        CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, NULL, 0, 0)) {

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            hBitmap = (HBITMAP) LoadImageA(NULL, (LPCSTR) BMP_PATH, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
            break;
        case WM_PAINT:
            ShowBitmap(hWnd, offsetX, offsetY);
            break;
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_DOWN:
                    offsetY += DELTA;
                    InvalidateRect(hWnd, NULL, TRUE);
                    break;
                case VK_UP:
                    offsetY -= DELTA;
                    InvalidateRect(hWnd, NULL, TRUE);
                    break;
                case VK_LEFT:
                    offsetX -= DELTA;
                    InvalidateRect(hWnd, NULL, TRUE);
                    break;
                case VK_RIGHT:
                    offsetX += DELTA;
                    InvalidateRect(hWnd, NULL, TRUE);
                    break;
            }
            break;
        case WM_MOUSEWHEEL:
            if (wParam & MK_SHIFT) {
                wheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
                for (; wheelDelta < 0; wheelDelta += WHEEL_DELTA) {
                    offsetX -= DELTA;
                    InvalidateRect(hWnd, NULL, TRUE);
                }
                for (; wheelDelta > 0; wheelDelta -= WHEEL_DELTA) {
                    offsetX += DELTA;
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            } else {
                wheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
                for (; wheelDelta < 0; wheelDelta += WHEEL_DELTA) {
                    offsetY += DELTA;
                    InvalidateRect(hWnd, NULL, TRUE);
                }
                for (; wheelDelta > 0; wheelDelta -= WHEEL_DELTA) {
                    offsetY -= DELTA;
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }
            break;
        case WM_LBUTTONDOWN:
            offsetX = LOWORD(lParam);
            offsetY = HIWORD(lParam);
            isLeftButtonDown = true;
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        case WM_LBUTTONUP:
            isLeftButtonDown = false;
        case WM_MOUSEMOVE:
            if (isLeftButtonDown) {
                offsetX = LOWORD(lParam);
                offsetY = HIWORD(lParam);
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;
        case WM_SIZE:
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void ShowBitmap(HWND hWnd, int &offsetX, int &offsetY) {

    BITMAP bitmap;

    GetObject(hBitmap, sizeof(bitmap), &bitmap);

    HDC winDC = GetDC(hWnd);
    HDC memDC = CreateCompatibleDC(winDC);
    HBITMAP oldBmp = (HBITMAP) SelectObject(memDC, hBitmap);
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;

    CorrectOffset(offsetX, offsetY, clientWidth, clientHeight);
    SetStretchBltMode(winDC, HALFTONE);
    StretchBlt(winDC, offsetX, offsetY,
               clientWidth / SCALE, clientHeight / SCALE,
               memDC, 0, 0,
               bitmap.bmWidth, bitmap.bmHeight, SRCAND);
    SelectObject(memDC, oldBmp);

    DeleteDC(memDC);

    ReleaseDC(hWnd, winDC);
}

void CorrectOffset(int &offsetX, int &offsetY, int clientWidth, int clientHeight) {
    if (offsetX < 0) {
        offsetX = DELTA + DELTA_PUSH;
    } else if (offsetX + clientWidth / SCALE > clientWidth) {
        offsetX = clientWidth - DELTA - DELTA_PUSH - clientWidth / SCALE;
    }
    if (offsetY < 0) {
        offsetY = DELTA + DELTA_PUSH;
    } else if (offsetY + clientHeight / SCALE > clientHeight) {
        offsetY = clientHeight - DELTA - DELTA_PUSH - clientHeight / SCALE;
    }
}


