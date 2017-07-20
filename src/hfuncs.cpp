#include <windows.h>

HBITMAP Icon2Bitmap(HICON icon)
{
    HDC dc = GetDC(0);
    HDC mdc = CreateCompatibleDC(dc);
    HBITMAP bitmap = CreateCompatibleBitmap(dc, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
    HGDIOBJ old = SelectObject(mdc, bitmap);
    DrawIcon(mdc, 0, 0, icon);
    SelectObject(mdc, old);
    DeleteDC(mdc);
    ReleaseDC(0, dc);
    return bitmap;
}
