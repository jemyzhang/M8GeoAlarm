#include "M8GeoAlarm.h"
#include "mz_commonfunc.h"
#include "ui_reminder.h"
#include <MotorVibrate.h>

#include "resource.h"
using namespace MZ_CommonFunc;
// The global variable of the application.
M8GeoAlarmApp theApp;
wchar_t ini_path[256];
wchar_t ini_reminder[256];
GeoReminder georeminder;

ImagingHelper *pimg[IDB_PNG_END - IDB_PNG_BEGIN + 1];
ImagingHelper *imgArrow;

void M8GeoAlarmApp::loadImageRes(){
	HINSTANCE resHandle = MzGetInstanceHandle();
	for(int i = 0; i < sizeof(pimg) / sizeof(pimg[0]); i++){
		pimg[i] = ImagingHelper::GetImageObject(
			resHandle, 
			IDB_PNG_BEGIN + i, true);
	}
	HINSTANCE MzresHandle = GetMzResModuleHandle();
	imgArrow = ImagingHelper::GetImageObject(MzresHandle, MZRES_IDR_PNG_ARROW_RIGHT, true);
}

BOOL M8GeoAlarmApp::Init() {
    // Init the COM relative library.
    CoInitializeEx(0, COINIT_MULTITHREADED);

	// 载入提醒
	georeminder.loadReminderList();

	// 判断是否是提醒调用
	LPWSTR str = GetCommandLine();
//	str = L"AppRunToHandleNotification 0x360000B1";
	wchar_t prestr[1024];
	if(lstrlen(str)){
		//处理小区
		int lac,cid;
		georeminder.getATLocalInfo(lac,cid);
		ReminderInfo_ptr p = georeminder.checkReminder(lac,cid);

		wchar_t tmp[1024];
		if(p){
			wsprintf(prestr,L"\n%s\n%s",p->name.C_Str(),C::restoreWrap(tmp,p->text.C_Str()));
			MzSetVibrateOn(MZ_VIBRATE_ON_TIME,MZ_VIBRATE_OFF_TIME);
			while(MzMessageBoxEx(0, prestr, L"Exit", MB_OK) != 1);
			MzSetVibrateOff();
		}
		HWND pWnd = isRuning();
		if(pWnd)
		{
			//运行时，通过主界面判断程序是否启用
			PostMessage(pWnd,MZ_WM_UI_REMINDER_SETNEW,NULL,NULL);
			if(p) PostMessage(pWnd,MZ_WM_UI_REMINDER_REFRESH,NULL,NULL);	//更新列表
		}else{
			georeminder.setNextCheckInterval(10);
		}
		PostQuitMessage(0);
		return true;
	}
	//正常启动程序
	//检测程序是否已经运行
	HWND pWnd = isRuning();
	if(pWnd)
	{
		SetForegroundWindow(pWnd);
		PostMessage(pWnd,WM_NULL,NULL,NULL);
		PostQuitMessage(0);
		return true; 
	}
	//载入图片
	loadImageRes();
	// Create the main window
	RECT rcWork = MzGetWorkArea();
	m_MainWnd.Create(rcWork.left, rcWork.top, RECT_WIDTH(rcWork), RECT_HEIGHT(rcWork), 0, 0, 0);
	m_MainWnd.Show();

    // return TRUE means init success.
    return TRUE;
}