//
// Created 3 years ago
// Code
// Revisions 2
// NtSetInformationThread hiding
//
#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

typedef enum _THREADINFOCLASS {
	ThreadBasicInformation,
	ThreadTimes,
	ThreadPriority,
	ThreadBasePriority,
	ThreadAffinityMask,
	ThreadImpersonationToken,
	ThreadDescriptorTableEntry,
	ThreadEnableAlignmentFaultFixup,
	ThreadEventPair,
	ThreadQuerySetWin32StartAddress,
	ThreadZeroTlsCell,
	ThreadPerformanceCount,
	ThreadAmILastThread,
	ThreadIdealProcessor,
	ThreadPriorityBoost,
	ThreadSetTlsArrayAddress,
	ThreadIsIoPending,
	ThreadHideFromDebugger
} THREADINFOCLASS;

// Will disable all debug notifications and exceptions from the thread.
NTSTATUS DisableDebuggingThread(HANDLE hThread) {
	typedef NTSTATUS (NTAPI *pNtSetInformationThread)(HANDLE,UINT,PVOID,ULONG);

	pNtSetInformationThread xNtSetInformationThread = (pNtSetInformationThread)GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")),"NtSetInformationThread");
	if(!xNtSetInformationThread)
		return FALSE;
	else
		return xNtSetInformationThread(hThread, ThreadHideFromDebugger, NULL, NULL);
}

void somethread(void* nul) {
	DebugBreak();
	return;
}

int main(void) {
	HANDLE hThread = NULL;
	hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&somethread, NULL, CREATE_SUSPENDED, NULL);
	assert(hThread);
	DisableDebuggingThread(hThread);
	ResumeThread(hThread);
	return 0;
}

.
Styling with Markdown is supported

    Contact GitHub API Training Shop Blog About 

    © 2016 GitHub, Inc. Terms Privacy Security Status Help 

