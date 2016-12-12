/*
	&ti-debugging using HeapSetInformation
	by ~vivid
	Apparently it's documented (sparingly) right here:
		http://msdn.microsoft.com/en-us/library/aa366750.aspx
*/

#include <Windows.h>
#include <stdio.h>

#define HCI HeapCompatibilityInformation // 80 columns... ;D

int main(void) {
	ULONG uHeapInfo = 2;
	if(!HeapSetInformation(GetProcessHeap(), HCI, &uHeapInfo, sizeof(ULONG))) {
		printf("Debugger is attached!");
	} else {
		printf("No debugger found.");
	}
	return 0;
}
