#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define CHANCE 25
#define INTERVAL 5000

HANDLE get_drive_handle(const WCHAR drive)
{
    WCHAR tmp[8];
    HANDLE handle;
    swprintf(tmp, 8, L"\\\\.\\%c:", drive);

    handle = CreateFileW(tmp,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_READONLY,
        0);

    return handle;
}

void open_cdrom(const HANDLE handle)
{
    DWORD ret_size = 0;

    DeviceIoControl(handle, FSCTL_LOCK_VOLUME, 0, 0, 0, 0, &ret_size, 0);
    DeviceIoControl(handle, FSCTL_DISMOUNT_VOLUME, 0, 0, 0, 0, &ret_size, 0);
    DeviceIoControl(handle, IOCTL_STORAGE_EJECT_MEDIA, 0, 0, 0, 0, &ret_size, 0);
}

void close_cdrom(const HANDLE handle)
{
    DWORD ret_size = 0;

    DeviceIoControl(handle, IOCTL_STORAGE_LOAD_MEDIA, 0, 0, 0, 0, &ret_size, 0);
}

int main()
{
#ifndef _DEBUG
    ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

    WCHAR cdroms[sizeof(DWORD) * 8 - 1];
    int cdrom_count = 0;

    DWORD drives = GetLogicalDrives();

    wchar_t letter = L'A';
    for (int i = 0; i < sizeof(DWORD) * 8; i++)
    {
        if (drives & 1 << i)
        {
            WCHAR path[8];
            swprintf(path, 8, L"\\\\.\\%c:\\", letter);

            UINT type = GetDriveTypeW(path);
            if (type == DRIVE_CDROM)
            {
#ifdef _DEBUG
                wprintf(L"Found CD-ROM Drive at %c:\\\n", letter);
#endif

                cdroms[cdrom_count] = letter;
                cdrom_count++;
            }
        }

        letter++;
    }

    HANDLE *cdrom_handles = malloc(sizeof(HANDLE) * cdrom_count);
    for (int i = 0; i < cdrom_count; i++)
    {
        cdrom_handles[i] = get_drive_handle(cdroms[i]);
    }

#ifdef _DEBUG
    int x = 0;
    while (x < 10)
#else
    while (1)
#endif
    {
        for (int i = 0; i < cdrom_count; i++)
        {
            int val = (int)((((double)rand() / RAND_MAX) * 100) / CHANCE);
            if (val == 0)
            {
#ifdef _DEBUG
                wprintf(L"Opening CD-ROM at %c:\\\n", cdroms[i]);
#endif
                open_cdrom(cdrom_handles[i]);
                Sleep(2000);
                close_cdrom(cdrom_handles[i]);
            }
        }
#ifdef _DEBUG
        x++;
#endif
        Sleep(INTERVAL);
    }

    for (int i = 0; i < cdrom_count; i++)
    {
        CloseHandle(cdrom_handles[i]);
    }
}
