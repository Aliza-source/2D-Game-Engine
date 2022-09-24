#pragma once
class SystemTimer {
public:
	_NtQueryTimerResolution GetResolution;

	BOOL SetResolution(UINT time) {
			MMRESULT timer = timeBeginPeriod(time);
			if (timer == TIMERR_NOCANDO) {
				MessageBoxW(NULL, L"Unable to set the global timer resolution!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
				return FALSE;
			}
			return TRUE;
	};

	BOOL Initialize() {
		HMODULE ntDllModuleHandle;
		if ((ntDllModuleHandle = GetModuleHandleA("ntdll.dll")) == NULL) {
			MessageBoxW(NULL, L"Unable to load ntdll.dll", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return FALSE;
		}
		if ((GetResolution = (_NtQueryTimerResolution)GetProcAddress(ntDllModuleHandle, "NtQueryTimerResolution")) == NULL) {
			MessageBoxW(NULL, L"Unable to find the NtQueryTimerResolution function in ntdll.dll", L"Error!", MB_ICONEXCLAMATION | MB_OK);
			return FALSE;
		}
		return TRUE;
	};
};