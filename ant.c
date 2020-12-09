
#include <winsock2.H>
#include <windows.h>
#include <tlhelp32.h>
#include <mstcpip.h>
#include <stdio.h>
#include <unistd.h>


/******************************************************************************/

#if 0
void hex_dump(char *str, const void *buf, int size)
{
	int i, j;
	const unsigned char *ubuf = buf;

	if(str)
		printf("%s:\n", str);

	for(i=0; i<size; i+=16){
		printf("%4x: ", i);
		for(j=0; j<16; j++){
			if((i+j)<size){
				printf(" %02x", ubuf[j]);
			}else{
				printf("   ");
			}
		}
		printf("  ");
		for(j=0; j<16&&(i+j)<size; j++){
			if(ubuf[j]>=0x20 && ubuf[j]<=0x7f){
				printf("%c", ubuf[j]);
			}else{
				printf(".");
			}
		}
		printf("\n");
		ubuf += 16;
	}
	printf("\n");
}
#endif


int hook_import(void *proc_base, char *lib_name, char *func_name, void **old_func, void *new_func)
{
	IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)proc_base;
	IMAGE_OPTIONAL_HEADER* pOpNtHeader = (IMAGE_OPTIONAL_HEADER*)((BYTE*)proc_base + pDosHeader->e_lfanew + 24);
	IMAGE_IMPORT_DESCRIPTOR* pImportDesc = (IMAGE_IMPORT_DESCRIPTOR*)((BYTE*)proc_base + pOpNtHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	BOOL bFindDll = FALSE;
	while(pImportDesc->FirstThunk) {
		char* imp_lib_name = (char*)((BYTE*)proc_base + pImportDesc->Name);
		//printf("imp_lib_name: %s\n", imp_lib_name);
		if(stricmp(imp_lib_name, lib_name) == 0) {
			bFindDll = TRUE;
			break;
		}
		pImportDesc++;
	}

	if(!bFindDll)
		return -1;


	int n = 0;
	IMAGE_THUNK_DATA* pThunk = (IMAGE_THUNK_DATA*)((BYTE*)proc_base + pImportDesc->OriginalFirstThunk);
	while(pThunk->u1.Function){
		char* imp_func_name = (char*)((BYTE*)proc_base + pThunk->u1.AddressOfData+2);
		PDWORD64 p_func_addr = (DWORD64*)((BYTE*)proc_base + pImportDesc->FirstThunk) + n;
		//printf("   func_name: %I64x %s\n", *p_func_addr, imp_func_name);

		if(strcmp(imp_func_name, func_name)==0){
			if(old_func){
				*old_func = (void*)*p_func_addr;
			}

			DWORD old_prot = 0;
			VirtualProtect(p_func_addr, sizeof(void*), PAGE_READWRITE, &old_prot);  
			*p_func_addr = (DWORD64)new_func;
			VirtualProtect(p_func_addr, sizeof(void*), old_prot, &old_prot);  

			return 0;
		}

		n += 1;
		pThunk += 1;
	}

	return -1;
}



/******************************************************************************/


LRESULT (*orig_kbdhook)(int code, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK kbdhook_cb(int code, WPARAM wParam, LPARAM lParam)
{
	if(code>=HC_ACTION){
		DWORD vkey = ((KBDLLHOOKSTRUCT*)lParam)->vkCode;
		if(wParam==WM_SYSKEYDOWN || wParam==WM_SYSKEYUP){
			if(vkey==0xc0){
				// Alt + `
				return CallNextHookEx(0, code, wParam, lParam);
			}
		}
	}

	return orig_kbdhook(code, wParam, lParam);
}


/******************************************************************************/


HHOOK WINAPI hook_SetWindowsHookExW(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId)
{
	if(idHook==WH_KEYBOARD_LL){
		orig_kbdhook = lpfn;
		lpfn = (HOOKPROC)kbdhook_cb;
	}

	return SetWindowsHookExW(idHook, lpfn, hmod, dwThreadId);
}


HMODULE WINAPI hook_LoadLibraryW(LPCWSTR lpLibFileName)
{
	HMODULE hModule = LoadLibraryW(lpLibFileName);

	if(lstrcmpiW(lpLibFileName, L"mstscax.dll")==0){
		hook_import(hModule, "user32.dll", "SetWindowsHookExW", NULL, hook_SetWindowsHookExW);
	}

	return hModule;
}


/******************************************************************************/


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  reason, LPVOID lpReserved)
{
	if(reason == DLL_PROCESS_ATTACH) {
#if 0
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		printf("\n\nInjectted! PID=%d TID=%d\n\n", GetCurrentProcessId(), GetCurrentThreadId());
#endif
		hook_import(GetModuleHandle(NULL), "kernel32.dll", "LoadLibraryW", NULL, hook_LoadLibraryW);
	}

	return TRUE;
}


