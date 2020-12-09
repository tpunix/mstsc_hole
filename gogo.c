
#include <windows.h>
#include <psapi.h>

#include <stdio.h>
#include <unistd.h>


STARTUPINFO si;
PROCESS_INFORMATION pi;

int create_proc(char *proc_name, int argc, char *argv[])
{
	char cmd_line[256], *p;
	int i;

	p = cmd_line;
	p += sprintf(p, "%s ", proc_name);
	for(i=1; i<argc; i++){
		p += sprintf(p, "%s ", argv[i]);
	}

	memset(&pi, 0, sizeof(pi));
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	printf("CreateProcess ... %s\n", cmd_line);
	int retv = CreateProcess(proc_name, cmd_line, NULL, NULL,
			//FALSE, CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP | CREATE_SUSPENDED,
			FALSE, CREATE_NEW_PROCESS_GROUP | CREATE_SUSPENDED,
			NULL, NULL, &si, &pi);

	if(retv==0){
		printf("CreatePocess failed! %ld\n", GetLastError());
		return -1;
	}

	return pi.dwProcessId;
}


int main(int argc, char *argv[])
{
	DWORD ThreadId;
	int pid;

	char cpath[240];
	GetModuleFileName(NULL, cpath, sizeof(cpath));
	char *p = strrchr(cpath, '\\');
	if(p) *p = 0;


	char lpszLibName[256];
	sprintf(lpszLibName, "%s\\ant.dll", cpath);


	pid = create_proc("c:\\windows\\system32\\mstsc.exe", argc, argv);
	if(pid<0)
		return -1;

	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if(hProcess==NULL){
		printf("Failed to open process %d!\n", pid);
		return -1;
	}

    LPSTR lpszRemoteFile = (LPSTR)VirtualAllocEx(hProcess, NULL, strlen(lpszLibName) + 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if(lpszRemoteFile==NULL){
		printf("Failed to remote alloc!\n");
		return -1;
	}
	WriteProcessMemory(hProcess, lpszRemoteFile, (PVOID)lpszLibName, strlen(lpszLibName) + 1, NULL);

	LPTHREAD_START_ROUTINE pfnThreadRtn = (LPTHREAD_START_ROUTINE)GetProcAddress( GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	if(pfnThreadRtn==NULL){
		printf("Failed to get LoadLibraryA!\n");
		return -1;
	}

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, lpszRemoteFile, 0, &ThreadId);
	if(hThread==NULL){
		printf("Failed to create remote thread!\n");
		return -1;
	}

	usleep(100000);
	ResumeThread(pi.hThread);

	CloseHandle(hProcess);
	return 0;
}

