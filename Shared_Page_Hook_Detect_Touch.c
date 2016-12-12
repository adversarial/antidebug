// Created 3 years ago
// Code
// Revisions 1
// Edited shared page detection
/*
	// Another &nti trick using WinAPI [passive]
	// by ~netrev
	//
	// Check for hooks by using the Shared bit in pages by calling QueryWorkingSet[Ex]()
	// Since [statically] [or whatever it's called] loaded dlls such as kernel32
	// have the COPYONWRITE protection by default, we can check if it's been tampered with.
	// 
	// Needless to say anything with kernel-space access will fuck this up. But hey, it's about progressively armouring
	//
	// also asm has ruined me with jmps
	//
*/

#include <Windows.h>
#include <Psapi.h>

BOOL AdIsPageShared(void* pVirtualAddr) {
	PSAPI_WORKING_SET_INFORMATION *ppwsi = NULL;
	DWORD_PTR dwVirtualPage = NULL;
	MEMORY_BASIC_INFORMATION mbi;
	unsigned int ecx;
	SYSTEM_INFO	si;
	BOOL bWow64,
		 bRet;

  // MSDN requires that the page be locked either by writing (atomically?) or via VirtualLock
  // I'm unsure of the usage of mbi.BaseAddress or mbi.AllocationBase here. Leaning towards BaseAddress as it is rounded to $1000
  // fuck it, using AllocationBase as hModule substitute though seeing as the loader... well allocated it at that base
	VirtualQuery(pVirtualAddr, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
	if(~mbi.Protect & PAGE_READONLY && ~mbi.Protect & PAGE_READWRITE)
		return FALSE;	// fucking idiot it has to be writecopy to begin with
	VirtualLock(mbi.BaseAddress, mbi.RegionSize);

  // what the fuck windows
  // you need to account for the memory you allocate to check the allocated memory
  // also that number constantly updates, however fuck it we only want an existing page
	ppwsi = (PSAPI_WORKING_SET_INFORMATION*)malloc(sizeof(PSAPI_WORKING_SET_INFORMATION));
	if(QueryWorkingSet(GetCurrentProcess(), ppwsi, sizeof(PSAPI_WORKING_SET_EX_INFORMATION)) == FALSE) {
		if(GetLastError() != ERROR_BAD_LENGTH) {
			goto retfalse;
		}
  rerealloc:
		ppwsi = (PSAPI_WORKING_SET_INFORMATION*)realloc(ppwsi, (ppwsi->NumberOfEntries * 1.25) * sizeof(PSAPI_WORKING_SET_INFORMATION));
		if(!QueryWorkingSet(GetCurrentProcess(), ppwsi, sizeof(PSAPI_WORKING_SET_INFORMATION) * ppwsi->NumberOfEntries))
			if(GetLastError() != ERROR_BAD_LENGTH) {
				goto retfalse;
			} else
				goto rerealloc;	// possible infinite loop here. realloc will account for pages that are being added
	}
  // find our page using some simple maths that I wish I had been born 10 years earlier to learn segmentation/etc...
  // shit man, I had to google this because I had no idea page != virtual allocation base. That's how much magic computers do now, or I'm an idiot.
  // VirtualPage = VirtualAddr / pageSize (truncated)
  // VirtualOff  = VirtualAddr % pageSize
	IsWow64Process(GetCurrentProcess(), &bWow64);
	if(bWow64)
		GetNativeSystemInfo(&si);
	else
		GetSystemInfo(&si);
	dwVirtualPage = (DWORD_PTR)pVirtualAddr / si.dwPageSize;

	for(ecx = ppwsi->NumberOfEntries; ecx; --ecx) {
		if((DWORD_PTR)ppwsi->WorkingSetInfo[ecx].VirtualPage == dwVirtualPage)
			goto pagefound;		// fuck yeah! fuck yeah jmps!
	}

  // land here if we didn't find the page (how did that happen?)
  // not sure how to handle this so we'll just return false
  // this will also be a default error place
  retfalse:
	VirtualUnlock(mbi.BaseAddress, mbi.RegionSize);
	free(ppwsi);
	return FALSE;

  pagefound:
	bRet = ppwsi->WorkingSetInfo[ecx].Shared;
	VirtualUnlock(mbi.BaseAddress, mbi.RegionSize);
	free(ppwsi);
	return bRet;
}
