#include "ui_main.h"
#include "ui_reminder.h"

#define VER_STRING L"1.00"
#ifdef MZFC_STATIC
#define BUILD_STRING L"20090527(静态编译)"
#else
#define BUILD_STRING L"20090527(动态编译)"
#endif
#define APPNAME L"地理提醒"
#ifdef _DEBUG
#define CONFIG_INI L"Program Files\\M8GeoAlarm\\M8GeoAlarm.ini"
#else
#define CONFIG_INI L"\\Disk\\Programs\\M8GeoAlarm\\M8GeoAlarm.ini"
#endif
// Application class derived from CMzApp
extern wchar_t ini_path[256];
extern wchar_t ini_reminder[256];
extern GeoReminder georeminder;
extern ImagingHelper *pimg[];
extern ImagingHelper *imgArrow;


class M8GeoAlarmApp : public CMzApp {
public:
    // The main window of the app.
    Ui_MainWnd m_MainWnd;

	void loadImageRes();
    // Initialization of the application
    virtual BOOL Init();
	HWND isRuning(){
		//正常启动程序
		//检测程序是否已经运行
		HANDLE m_hCHDle = CreateMutex(NULL,true,L"M8GeoAlarm");
		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			HWND pWnd=FindWindow(m_MainWnd.GetMzClassName(),NULL);
			return pWnd; 
		}
		return 0;
	}
};
