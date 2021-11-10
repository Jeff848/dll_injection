#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

//Given process name, output process ID
DWORD GetProcId(const char* procName)
{
    DWORD procId = 0;
    //Create a "snapshot" of the processes
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        //procEntry is a struct that will receive each procEntry from the snapshot
        PROCESSENTRY32 procEntry;
        procEntry.dwSize = sizeof(procEntry);

        //procEntry receives the first process from the snapshot
        if (Process32First(hSnap, &procEntry))
        {
            do
            {
                //stricmp (string insensitive compare) (case-insensitive) 
                //compare between the procEntry name and the target process name (our input)
                if (!_stricmp(procEntry.szExeFile, procName))
                {
                    //grab the process ID
                    procId = procEntry.th32ProcessID;
                    break;
                }

            } while (Process32Next(hSnap, &procEntry)); //put the next process from the snapshot into procEntry
        }
    }
    CloseHandle(hSnap);
    return procId;
}

int main()
{
    const char* dllPath = ""; // Example: "C:\\Users\\<username>\\source\\repos\\aimbot_boilerplate\\Debug\\aimbot_tutorial.dll"
    const char* procName = "csgo.exe"; // Example: "csgo.exe"
    DWORD procId = 0;

    while (!procId)
    {
        procId = GetProcId(procName);
        Sleep(30);
    }

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procId);

    if (hProc && hProc != INVALID_HANDLE_VALUE)
    {

        //Allocate memory in an external process
        //Get section of memory in target process and allocate it with read/write permissions
        void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

        if (loc)
        {
            //Write the DLL *path* (not the DLL itself) into memory at the location above
        //WriteProcessMemory(handleToGame, addressToWriteTo, variableContainingValueToWrite, sizeof(variableContainingValueToWrite), NULL);
            WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0);
        }

        //Call LoadLibraryA with argument of loc which is the location with the DLL path stored there
        //This will map it into memory and run it! 
        HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);

        //Close the thread
        if (hThread)
        {
            CloseHandle(hThread);
        }
    }

    //Close the process handle
    if (hProc)
    {
        CloseHandle(hProc);
    }
    return 0;
}