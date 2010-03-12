#pragma once

#include "cMzCommon.h"
using namespace cMzCommon;
#include <vector>
using std::vector;

typedef enum tagRepeatType{
    REPEAT_NONE		=	0,
    REPEAT_1TIME	=	1,
    REPEAT_MINUTE	=	2,
    REPEAT_HOUR		=	3,
    REPEAT_DAY		=	4,
    REPEAT_WEEK		=	5,
}REPEAT_t;

typedef enum tagReminderType{
    REMINDER_TEXT   =   0,
    REMINDER_SMS    =   1,
    REMINDER_EXE    =   2,
}REMINDER_t;

typedef struct tagReminderInfo{
    tagReminderInfo(){
            MCC = 0; MNC = 0; LAC = 0; CID = 0;
            type = REPEAT_NONE;
            text = 0; name = 0;
            isEna = false;
            //TODO
            nextAlarm = L"00000000000000";
            isRemoved = false;
    }
    ~tagReminderInfo(){
        if(name){
            delete name;
            name = 0;
        }
        if(text){
            delete text;
            text = 0;
        }
        if(nextAlarm){
            delete nextAlarm;
            nextAlarm = 0;
        }
    }
    DWORD MCC;    //mobile country code
    DWORD MNC;    //mobile network code
    DWORD LAC;	//event id(handle)
    DWORD CID;	//record id
    bool isEna;	//isEnable or not
    bool isRemoved;
    REPEAT_t type;
    LPWSTR name;		//local name
    LPWSTR text;		//reminder text
    LPWSTR nextAlarm;	//next alarm time string: yyyymmddhhMMss
}ReminderInfo_t,*LPReminderInfo;

//提醒读写类
class GeoReminder
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
    void updateReminderList(LPReminderInfo r);
public:
    //如果循环打开，设置下一个循环点
    //	bool setNextReminder(LPReminderInfo r);
    HANDLE setNextCheckInterval(int interval = 10); //s
public:
    //新增或更新配置
    bool addReminder(LPReminderInfo r);
    bool updateReminder(LPReminderInfo r);
    LPReminderInfo deleteReminder(LPReminderInfo r);
public:
    int getReminderIndex(int lac, int cid);
    LPReminderInfo getReminderByIndex(int idx);
    LPReminderInfo getReminder(int lac, int cid);
    LPReminderInfo checkReminder(int lac, int cid);
    LPReminderInfo checkCurrentReminder();
    
public:
    //清除所有提醒
    void clearReminder();
public:
    wchar_t* rinipath;
    vector<LPReminderInfo> reminders;
};

//提醒设置界面
class Ui_GeoReminderWnd : public CMzWndEx 
{
    MZ_DECLARE_DYNAMIC(Ui_GeoReminderWnd);
public:
    Ui_GeoReminderWnd(void);
    ~Ui_GeoReminderWnd(void);
    void setupReminderInfo(LPReminderInfo preminder){
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
    UiEdit m_EdtNote;	//提醒内容
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
    LPReminderInfo r;
};
