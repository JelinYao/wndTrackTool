// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include <algorithm>
#include <string>
#include <atlstr.h>
#include <Psapi.h>
#pragma comment(lib, "psapi.lib")


using std::wstring;
#define WM_GETNEW_WND	WM_USER+100
#define WM_HOTKEY_BASE	100


inline bool operator != (const RECT& rc1, const RECT& rc2)
{
	return ((rc1.left!=rc2.left)
		|| (rc1.top!=rc2.top)
		|| (rc1.right!=rc2.right)
		|| (rc1.bottom!=rc2.bottom));
}

class CMainDlg : public CDialogImpl<CMainDlg>
{
public:
	static bool s_bThreadSwitch;
	static bool s_bRunning;
	static HBRUSH s_hFrameBrush;
	HWND m_hEditWnd;
	enum { IDD = IDD_MAINDLG };
	CMainDlg()
		:m_hEditWnd(NULL)
	{

	}
	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_GETNEW_WND, OnGetNewWindow)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_HOTKEY, OnHotKey)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// center the dialog on the screen
		CenterWindow();
		// set icons
		HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
		SetIcon(hIconSmall, FALSE);
		m_hEditWnd=::GetDlgItem(m_hWnd, IDC_EDIT1);
		::SetDlgItemText(m_hWnd, IDC_STATUS_BAR, L"窗口捕获未开启，Alt+B立即开启");
		//注册开始捕获热键
		::RegisterHotKey(m_hWnd, WM_HOTKEY_BASE+0, MOD_ALT, 0x42);
		//注册停止捕获热键
		::RegisterHotKey(m_hWnd, WM_HOTKEY_BASE+1, MOD_ALT, 0x53);
		s_hFrameBrush = ::CreateSolidBrush(0xff0000);
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE)|WS_EX_TOPMOST);
		return TRUE;
	}

	LRESULT OnGetNewWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if ( NULL==m_hEditWnd )
			return 0;
		HWND hWnd = (HWND)wParam;
		if (hWnd == m_hEditWnd)
		{
			::SetWindowText(m_hEditWnd, L"");
			return 0;
		}
		WCHAR szText[MAX_PATH+1]={0};
		LRESULT lSize=::SendMessage(hWnd, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szText);
		if ( lSize==0 )
			wcscpy(szText, L"窗口标题为空");
		WCHAR szClassName[128] = { 0 };
		::GetClassName(hWnd, szClassName, 100);
		CComBSTR url;
		if ( wcsncmp(szClassName, L"Internet Explorer_Server", 24) == 0 )
		{//此窗口为内嵌IE控件
			UINT uMsg=::RegisterWindowMessage(L"WM_HTML_GETOBJECT");
			DWORD_PTR dwRetPtr;
			LRESULT lRet=::SendMessageTimeout(hWnd, uMsg, 0, 0, SMTO_ABORTIFHUNG, 1000, &dwRetPtr);
			CComQIPtr<IHTMLDocument> docPtr;
			HRESULT hr=::ObjectFromLresult((LRESULT)dwRetPtr, IID_IHTMLDocument, 0, (void**)&docPtr);
			if ( SUCCEEDED(hr) && docPtr )
			{
				CComQIPtr<IDispatch> scriptPtr;
				hr=docPtr->get_Script(&scriptPtr);
				if ( SUCCEEDED(hr) && scriptPtr )
				{
					CComQIPtr<IHTMLWindow2> htmlwndPtr=scriptPtr;
					CComPtr<IHTMLDocument2> pDoc2;
					hr=htmlwndPtr->get_document(&pDoc2);
					if (SUCCEEDED(hr) && pDoc2)
					{
						pDoc2->get_URL(&url);
					}
				}
			}
		}
		//枚举进程加载的所有DLL
		DWORD dwPid;
		DWORD dwTid = ::GetWindowThreadProcessId(hWnd, &dwPid);
		HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
		CString strDllList;
		if (hProcess)
		{
			DWORD dwNeeded = 0;
			if (EnumProcessModules(hProcess, NULL, 0, &dwNeeded) && dwNeeded > 0)
			{
				int nCount = dwNeeded / sizeof(HMODULE);
				HMODULE *pModules = new HMODULE[nCount];
				TCHAR szModName[MAX_PATH];
				if (EnumProcessModules(hProcess, pModules, dwNeeded, &dwNeeded))
				{
					for (int i = 0; i < nCount; i++)
					{
						if (GetModuleFileNameEx(hProcess, pModules[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
						{
							wstring modPath = szModName;
							strDllList.Append(L"\r\n");
							strDllList.Append(szModName);

						}
					}
				}
				delete[] pModules;
			}
			CloseHandle(hProcess);
		}

		CString strText;
		strText.Format(L"%s\r\n%s", szText, szClassName);
		if (url.Length() > 0)
		{
			strText.Append(L"\r\n");
			strText.Append(url);
		}
		strText += strDllList;
		::SendMessage(m_hEditWnd, WM_SETTEXT, 0, (LPARAM)(LPCWSTR)strText);
		bHandled=TRUE;
		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (s_hFrameBrush)
		{
			::DeleteObject(s_hFrameBrush);
			s_hFrameBrush = NULL;
		}
		::UnregisterHotKey(m_hWnd, WM_HOTKEY_BASE+0);
		::UnregisterHotKey(m_hWnd, WM_HOTKEY_BASE+1);
		s_bThreadSwitch = false;
		//等待线程退出
		while (s_bRunning)
			::Sleep(100);
		return 0;
	}

	LRESULT OnHotKey(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		switch (wParam)
		{
		case WM_HOTKEY_BASE + 0:
			BeginTrackWnd();
			break;
		case WM_HOTKEY_BASE + 1:
			StopTrackWnd();
			break;
		default:
			break;
		}
		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CSimpleDialog<IDD_ABOUTBOX, FALSE> dlg;
		dlg.DoModal();
		return 0;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: Add validation code 
		EndDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int nWidth = GET_X_LPARAM(lParam);
		int nHeight = GET_Y_LPARAM(lParam);
		::MoveWindow(m_hEditWnd, 0, 0, nWidth, nHeight-30, TRUE);
		::MoveWindow(::GetDlgItem(m_hWnd, IDC_STATUS_BAR), 0, nHeight-20, nWidth, 20, TRUE);
		return 0;
	}

	void BeginTrackWnd()
	{
		if (!s_bRunning)
		{
			s_bThreadSwitch=true;
			HANDLE hThread = ::CreateThread(NULL, 0, MouseThread, m_hWnd, 0, NULL);
			::CloseHandle(hThread);
			::SetDlgItemText(m_hWnd, IDC_STATUS_BAR, L"窗口捕获已经开启，获得IE链接后，Alt+S结束捕获");
		}
	}

	void StopTrackWnd()
	{
		if (s_bRunning)
		{
			s_bThreadSwitch = false;
			::SetDlgItemText(m_hWnd, IDC_STATUS_BAR, L"窗口捕获未开启，Alt+B立即开启");
		}
	}

	static DWORD __stdcall MouseThread(LPVOID lpParam)
	{
		s_bRunning=true;
		POINT ptCur = { 0,0 };
		HWND hMsgWnd = (HWND)lpParam;
		HWND hPreWnd = NULL;
		RECT rcWnd = { 0L, 0L, 0L, 0L }, rcPrev = { -1L, -1L, -1L, - 1L };
		HWND hDeskWnd = ::GetDesktopWindow();
		while (s_bThreadSwitch)
		{
			::GetCursorPos(&ptCur);
			HWND hWnd = ::WindowFromPoint(ptCur);
			if (NULL != hWnd && hPreWnd != hWnd)
			{
				::PostMessage(hMsgWnd, WM_GETNEW_WND, (WPARAM)hWnd, 0);
				hPreWnd = hWnd;
				if (rcPrev != rcWnd)
				{
					::InflateRect(&rcPrev, 10, 10);
					::InvalidateRect(hDeskWnd, NULL, TRUE);
					HDC hDC = ::GetDC(hDeskWnd);
					RECT rcWnd;
					::GetWindowRect(hWnd, &rcWnd);
					::FrameRect(hDC, &rcWnd, s_hFrameBrush);
					::ReleaseDC(hDeskWnd, hDC);
					rcPrev = rcWnd;
				}
			}
			Sleep(100);
		}
		s_bRunning = false;
		return 0;
	}
};

bool CMainDlg::s_bThreadSwitch = true;
bool CMainDlg::s_bRunning = false;
HBRUSH CMainDlg::s_hFrameBrush = NULL;