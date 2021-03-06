/* utils.cpp - contains the utilities to be used by NTPDRAPE-II */

#include "common.hpp"
#include <filesystem>


namespace Utils {

	namespace MIDI {
		UINT midiDeviceId;
		DWORD PlayMIDIFromPath(HWND hwnd, const wchar_t* path)
		{
			MCI_OPEN_PARMS mciOpenParms;
			mciOpenParms.lpstrDeviceType = L"sequencer";
			mciOpenParms.lpstrElementName = path;

			DWORD dwReturn;
			if ((dwReturn = mciSendCommand(0, MCI_OPEN, 
										 MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)(LPMCI_OPEN_PARMS)&mciOpenParms)))
				return (dwReturn);

			midiDeviceId = mciOpenParms.wDeviceID;

			MCI_STATUS_PARMS mciStatusParms;
			mciStatusParms.dwItem = MCI_SEQ_STATUS_PORT;

			if ((dwReturn = mciSendCommand(midiDeviceId, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPMCI_STATUS_PARMS)&mciStatusParms)))
			{
				mciSendCommand(midiDeviceId, MCI_CLOSE, 0, 0);
				return (dwReturn);
			}

			MCI_PLAY_PARMS mciPlayParms;
			mciPlayParms.dwCallback = (DWORD)hwnd;

			if ((dwReturn = mciSendCommand(midiDeviceId, MCI_PLAY, MCI_NOTIFY, (DWORD)(MCI_PLAY_PARMS*)&mciPlayParms)))
			{
				mciSendCommand(midiDeviceId, MCI_CLOSE, 0, 0);
				return (dwReturn);
			}

			return S_OK;
		}
	}
	
	namespace Resource
	{
		bool ExtractFromResource(const wchar_t* dest, LPCWSTR resName, DWORD CreateFileDisposition)
		{
			// Don't waste time if we already extracted
			if (std::filesystem::exists(dest))
				return true;

		    HRSRC rsrc = FindResource(nullptr, resName, RT_RCDATA);
		    if (!rsrc)
		        return false;
		        
		    DWORD resSize = SizeofResource(nullptr, rsrc);
		    if (!resSize)
		        return false;
		    
		    LPVOID rsrcData = LockResource(LoadResource(nullptr, rsrc));
		    if (!rsrcData)
		        return false;
	
		    HGLOBAL hglobal = GlobalAlloc(GMEM_MOVEABLE, resSize);
		    if (hglobal)
		    {
		        LPVOID extractBuffer = GlobalLock(hglobal);
		        if (extractBuffer)
		        {
		            memcpy(extractBuffer, rsrcData, resSize);

		            HANDLE file = CreateFileW(dest, GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CreateFileDisposition, FILE_ATTRIBUTE_NORMAL, nullptr);
		            if (file)
		            {
		                DWORD bytesWritten;
		                if(WriteFile(file, extractBuffer, resSize, &bytesWritten, nullptr)
		                   &&
		                   CloseHandle(file))
		                   return true; // Would this memleak???
		            }
		            GlobalUnlock(hglobal);
		        }
		        GlobalFree(hglobal);
		    }
		  	return false;
		}
	
		bool ExtractFromResource(const wchar_t* dest, LPCWSTR resName)
		{
			// Default to create new file
			return ExtractFromResource(dest, resName, CREATE_NEW);
		}
	}
}