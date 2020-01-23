#ifndef COMBOSEL_WIN_H_
#define COMBOSEL_WIN_H_


//Libraries: gdi32, comctl32
//Linker flags (gcc): -mwindows

#if defined(_WIN32) || defined(_WIN64)


#if defined(WINVER) || defined(_WIN32_WINNT)
	#if (WINVER < _WIN32_WINNT_WINXP) || (_WIN32_WINNT < _WIN32_WINNT_WINXP)
		#error Windows version to old
	#endif
#endif


#define COMBOSEL_RET_CANCEL -1
#define COMBOSEL_RET_CLOSE -2

////////////////////////////////////////////////////////////////////////////
//	ComboSel
//
//  Returns:
//		> 0		on failure
//		0		on success
//		< 0:	COMBOSEL_RET_CANCEL		Cancel-button
//				COMBOSEL_RET_CLOSE		Alt F4, Close Window, Menu->Exit
//
int ComboSel(const char *, const char *, const char **, int, int *);


#endif

#endif /* COMBOSEL_WIN_H_ */
