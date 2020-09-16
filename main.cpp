#include <iostream>
#include <windows.h>

/* 1. Изучение событийной архитектору Windows-приложений, механизма обработки сообщений, механизма перерисовки окна.

- Разработать программу, позволяющую передвигать с помощью клавиатуры и мыши спрайт (окрашенный прямоугольник или эллипс) внутри рабочей области окна.
- Обеспечить работу колесика мыши. Прокручивание двигает спрайт по вертикали. С удерживаемой клавишей Shift прокручивание колесика двигает спрайт по горизонтали.
- Заменить спрайт на картинку с непрямоугольным контуром.
- Придать спрайту движение с отскоком от границ окна.

*/

HBITMAP hBitmap;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void ShowBitmap(HWND hWnd, HBITMAP hBmp, int &offsetX, int &offsetY);

void CorrectOffset(int &offsetX, int &offsetY, RECT windowRect);

int offsetX = 0;
int offsetY = 0;
const int DELTA = 20;
const int SCALE = 5;
bool isLeftButtonDown = false;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR lpCmdLine,
                     int nCmdShow) {
    WNDCLASSEX wcex;
    HWND hWnd;
    MSG msg;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_DBLCLKS;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
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
        UpdateWindow(hWnd);
    }

    return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
                         WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            hBitmap = (HBITMAP) LoadImageA(NULL, (LPCSTR) ("test.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
            break;
        case WM_PAINT:
            ShowBitmap(hWnd, hBitmap, offsetX, offsetY);
            break;
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_DOWN:
                    offsetY += DELTA;
                    UpdateWindow(hWnd);
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
            int wheelDelta;
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
        case WM_SIZE:
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void ShowBitmap(HWND hWnd, HBITMAP hBmp, int &offsetX, int &offsetY) {
    PAINTSTRUCT paintStruct;

    BITMAP bitmap;
    HDC winDC = BeginPaint(hWnd, &paintStruct);
    GetObject(hBitmap, sizeof(bitmap), &bitmap);
    HDC memDC = CreateCompatibleDC(winDC);
    HBITMAP oldBmp = static_cast<HBITMAP>(SelectObject(memDC, hBmp));
    RECT windowRect = {0};
    GetWindowRect(hWnd, &windowRect);
    CorrectOffset(offsetX, offsetY, windowRect);
    StretchBlt(winDC, offsetX, offsetY,
               (windowRect.right - windowRect.left) / SCALE,
               (windowRect.bottom - windowRect.top) / SCALE,
               memDC, 0, 0,
               bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
    SelectObject(memDC, oldBmp);
    DeleteDC(memDC);
    EndPaint(hWnd, &paintStruct);
}

void CorrectOffset(int &offsetX, int &offsetY, RECT windowRect) {
    if (offsetX < 0) {
        offsetX = DELTA;
    } else if (offsetX + (windowRect.right - windowRect.left) / SCALE > (windowRect.right - windowRect.left)) {
        offsetX = (windowRect.right - windowRect.left) - DELTA - (windowRect.right - windowRect.left) / SCALE;
    }
    if (offsetY < 0) {
        offsetY = DELTA;
    } else if (offsetY + (windowRect.bottom - windowRect.top) / SCALE > (windowRect.bottom - windowRect.top)) {
        offsetY = (windowRect.bottom - windowRect.top) - DELTA - (windowRect.bottom - windowRect.top) / SCALE;
    }
}


