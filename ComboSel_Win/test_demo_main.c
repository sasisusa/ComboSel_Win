#include <windows.h>
#include <stdio.h>

#include "ComboSel_Win/ComboSel_Win.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int iRet, iSel;
	const char *asSel[] = {
		"65536    (2^16)", "131072   (2^17)", "262144   (2^18)", "524288   (2^19)", "1048576  (2^20)",
		"2097152  (2^21)", "4194304  (2^22)", "8388608  (2^23)", "16777216 (2^24)", "33554432 (2^25)"
		};

	iRet = ComboSel("ComboBox", "Select the number of elements:", asSel, sizeof(asSel)/sizeof(asSel[0]), &iSel);
	switch(iRet){
	case 0:
		printf("Selection: Nr. %d, %s\n", iSel, asSel[iSel]);
		break;
	case COMBOSEL_RET_CANCEL:
		puts("Canceled");
		break;
	case COMBOSEL_RET_CLOSE:
		puts("Closed");
		break;
	default:
		printf("Error: ComboSel ret:%d\n", iRet);
		break;
	}

	return 0;
}
