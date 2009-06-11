#pragma once

// include the MZFC library header file
#include <mzfc_inc.h>
#include "mz_commonfunc.h"
#include <list>

typedef enum RepeatType{
	REPEAT_NONE		=	0,
	REPEAT_1TIME	=	1,
	REPEAT_MINUTE	=	2,
	REPEAT_HOUR		=	3,
	REPEAT_DAY		=	4,
	REPEAT_WEEK		=	5,
}REPEAT_t;

typedef struct ReminderInfo{
	ReminderInfo(unsigned int LAC = 0, unsigned int cid = 0, bool e = false,
		REPEAT_t t = REPEAT_NONE, wchar_t* n = L"无地名", wchar_t* txt = L"无提醒文本"){
		LAC = LAC; CID = cid;
		type = t;
		text = txt;
		name = n;
		isEna = e;
		nextAlarm = L"00000000000000";
	}
	int LAC;	//event id(handle)
	int CID;	//record id
	bool isEna;	//isEnable or not
	REPEAT_t type;
	CMzString name;		//local name
	CMzString text;		//reminder text
	CMzString nextAlarm;	//next alarm time string: yyyymmddhhMMss
}ReminderInfo_t,*ReminderInfo_ptr;

//提醒读写类
class AtCommander
{
public:
	bool open(){
		bool ret = true;
		hCOM = CreateFile( L"COM9:" , 
			GENERIC_READ|GENERIC_WRITE , FILE_SHARE_READ|FILE_SHARE_WRITE , 
			NULL , OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL );
		if ( hCOM == INVALID_HANDLE_VALUE ){
			ret = false;
		}
		return ret;
	}
	bool close(){
		bool ret = CloseHandle(hCOM);
		if(ret){
			hCOM = INVALID_HANDLE_VALUE;
		}
		return ret;
	}
	bool send(char* cmd){
		bool ret = false;
		if(cmd == 0) return ret;

		DWORD dwBytes;
		char p = 0x0D;        //cr
		char q = 0x0A;        //lf
		size_t cmdlen = strlen(cmd);
		if(cmdlen == 0) return ret;

		WriteFile( hCOM, cmd , cmdlen , &dwBytes , NULL );
		if(dwBytes != cmdlen) { return ret; }
		WriteFile( hCOM, &p , 1 , &dwBytes , NULL );
		if(dwBytes != 1) {	return ret;	}
		WriteFile( hCOM, &q , 1 , &dwBytes , NULL );
		if(dwBytes != 1) {	return ret;	}
		ret = true;
		return ret;
	}
	DWORD receive(char* res, size_t len){
		if(res == 0 || len <= 0) return 0;
		DWORD dwBytes;
		ReadFile( hCOM , res , len , &dwBytes , NULL );
		return dwBytes;
	}
	HANDLE getComHandle(){
		return hCOM;
	}
public:
	AtCommander(){
		hCOM = INVALID_HANDLE_VALUE;
	}
	~AtCommander(){
		if(hCOM != INVALID_HANDLE_VALUE){
			close();
		}
	}
private:
	HANDLE hCOM;
};

class GeoInfo {
public:
	GeoInfo(){
		m_atCmdLock = false;
		_useSingleCmd = true;
		_batchCmdSent = false;
	}
	~GeoInfo(){
		if(!_useSingleCmd && _batchCmdSent){
			sendBatchEndCommand();
		}
	}
public:
	void setMethod(bool singlecmd){
		_useSingleCmd = singlecmd;
	}
	bool getMethod(){
		return _useSingleCmd;
	}
	bool getLocalInfo(int &lac, int &cid);
	bool sendSingleCommand();
	bool sendBatchStartCommand();
	bool sendBatchEndCommand();
private:
	bool _useSingleCmd;
	bool _batchCmdSent;
	AtCommander m_atCmd;
	bool m_atCmdLock;
};

class GeoReminder : public GeoInfo
{
public:
	GeoReminder(void);
	~GeoReminder(void);
public:
	HANDLE Notify(SYSTEMTIME st, HANDLE hNotification = (HANDLE)0);
public:
	//返回reminder数量
	void setReminderIniPath(wchar_t* p){
		rinipath = p;
	}
	void loadReminderList();
	void saveReminderList();
	void updateReminderList(ReminderInfo_ptr r);
public:
	//如果循环打开，设置下一个循环点
//	bool setNextReminder(ReminderInfo_ptr r);
	HANDLE setNextCheckInterval(int interval = 10); //s
public:
	//新增或更新配置
	bool addReminder(ReminderInfo_ptr r);
	bool updateReminder(ReminderInfo_ptr r);
	ReminderInfo_ptr deleteReminder(ReminderInfo_ptr r);
public:
	int getReminderIndex(int lac, int cid);
	ReminderInfo_ptr getReminderByIndex(int idx);
	ReminderInfo_ptr getReminder(int lac, int cid);
	ReminderInfo_ptr checkReminder(int lac, int cid);
public:
	//清除所有提醒
	void clearReminder();
public:
	wchar_t* rinipath;
	list<ReminderInfo_ptr> list_reminder;
};

//提醒设置界面
class Ui_GeoReminderWnd : public CMzWndEx 
{
	MZ_DECLARE_DYNAMIC(Ui_GeoReminderWnd);
public:
	Ui_GeoReminderWnd(void);
	~Ui_GeoReminderWnd(void);
	void setupReminderInfo(ReminderInfo_ptr preminder){
		r = preminder;
	}
	void setEnable(bool e){
		//m_BtnReminderSW.SetState(e ? MZCS_BUTTON_PRESSED : MZCS_BUTTON_NORMAL);
	}
	bool isEnabled(){
		return true;//(m_BtnReminderSW.GetState() == MZCS_BUTTON_PRESSED);
	}
private:
	void updateUi();
	bool checkDateText();
public:
	UiScrollWin m_ScrollWin;
    UiToolbar_Text m_Toolbar;	//确定
	//UiButton m_StaticDate;		//提醒日期
    UiSingleLineEdit m_EdtLAC;	//提醒小区号
    UiSingleLineEdit m_EdtCI;	//提醒基站号
    UiSingleLineEdit m_EdtLACName;	//名称
	//UiButtonEx m_BtnRepeat;		//重复按钮 ON/OFF
	//UiButtonEx m_BtnRepeatSW;	//重复按钮 ON/OFF
	UiMultiLineEdit m_EdtNote;	//提醒内容
	//UiButtonEx m_BtnReminder;
	//UiButtonEx m_BtnReminderSW;
protected:
    // Initialization of the window (dialog)
    virtual BOOL OnInitDialog();

    // override the MZFC command handler
    virtual void OnMzCommand(WPARAM wParam, LPARAM lParam);
    // override the MZFC window messages handler
    virtual LRESULT MzDefWndProc(UINT message, WPARAM wParam, LPARAM lParam);
private:
	ReminderInfo_ptr r;
};
