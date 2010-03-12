#include "ui_reminder.h"
#include <notify.h>
#include "ReadWriteIni.h"
#include "M8GeoAlarm.h"
#include "cellid_info.h"

MZ_IMPLEMENT_DYNAMIC(Ui_GeoReminderWnd)

#define MZ_IDC_TOOLBAR_REMINDER 101
#define MZ_IDC_Edit_HOUR 102
#define MZ_IDC_Edit_MIN 103
#define MZ_IDC_BTN_REPEAT 104
#define MZ_IDC_LIST_TYPE 105
#define MZ_IDC_BTN_REMINDER 106
#define MZ_IDC_S_DATE	107

#ifdef _DEBUG
#define DEFAULT_REMINDER_INI		L"Program Files\\M8Cash\\reminder.ini"
#else
#define DEFAULT_REMINDER_INI		L"\\Disk\\Programs\\M8Cash\\reminder.ini"
#endif

GeoReminder::GeoReminder()
{
    bool ret;
    //默认提醒配置文件路径
    wchar_t currpath[MAX_PATH];
    if(File::GetCurrentPath(currpath)){
        wsprintf(ini_reminder,L"%sreminder.ini",currpath);
    }else{
        wsprintf(ini_reminder,DEFAULT_REMINDER_INI);
    }
    //创建配置文件
    if(!File::FileExists(ini_reminder)){
        ret = IniCreateFile(ini_reminder);
    }
	CreateCellInfoInstance();
}

GeoReminder::~GeoReminder(){
	FinalizeCellInfoInstance();
}

HANDLE GeoReminder::Notify(SYSTEMTIME st, HANDLE hNotification)
{                
    TCHAR szExeName[MAX_PATH];  //需要运行的程序
    TCHAR szSound[MAX_PATH];    //需要播放的声音
    TCHAR Title[128]=L"标题";   //对话框标题，Ｍ８上没用
    TCHAR Text[128]=L"内容";    //对话框内容，Ｍ８上没用

    //取自身文件名
    ::GetModuleFileNameW(NULL,szExeName, MAX_PATH); 
    //同名音乐
    lstrcpy(szSound,szExeName);
    lstrcpy(szSound+ lstrlen(szSound) - 4, TEXT(".no"));//正常应该是．ｗａｖ，这里故意搞个不存在的文件，取消系统提醒声音，自己启动后根据情况播放。

    CE_USER_NOTIFICATION g_ceun; 
    memset (&g_ceun,0,sizeof(g_ceun));
    g_ceun.ActionFlags=(PUN_VIBRATE);
    g_ceun.pwszDialogTitle=Title;
    g_ceun.pwszDialogText=Text;
    g_ceun.pwszSound=szSound; 
    g_ceun.nMaxSound=sizeof(szSound);

    return CeSetUserNotification(hNotification,szExeName,&st,&g_ceun);
}

void GeoReminder::loadReminderList(){
    int size = 0;
    if(reminders.size()){
        clearReminder();
    }
    if(IniReadInt(L"ReminderInfo",L"nSize",(DWORD*)&size,ini_reminder) && ( size > 0)){
        int cnt = 0;
        for(int i = 0; i < size; i++){
            LPReminderInfo r = new ReminderInfo_t;
            wchar_t header[16];
            wsprintf(header,L"Reminder%d",cnt+1);
            IniReadInt(header,L"LAC",(DWORD*)&r->LAC,ini_reminder);
            IniReadInt(header,L"CID",(DWORD*)&r->CID,ini_reminder);
            IniReadInt(header,L"isEnable",(DWORD*)&r->isEna,ini_reminder);
            IniReadInt(header,L"RepeatType",(DWORD*)&r->type,ini_reminder);

            wchar_t* pdt;
            IniReadString(header,L"LocalName",&pdt,ini_reminder);
            C::newstrcpy(&r->name, pdt);
            delete [] pdt;

            IniReadString(header,L"EventText",&pdt,ini_reminder);
            C::newstrcpy(&r->text, pdt);
            delete [] pdt;

            IniReadString(header,L"NextAlarm",&pdt,ini_reminder);
            C::newstrcpy(&r->nextAlarm, pdt);
            delete [] pdt;

            reminders.push_back(r);
            cnt++;
        }
    }
}

void GeoReminder::saveReminderList(){
    File::DelFile(ini_reminder);	//删除旧文件
    IniCreateFile(ini_reminder);
    //写入配置大小
    IniWriteInt(L"ReminderInfo",L"nSize",reminders.size(),ini_reminder);
    //写入配置
    for(int i = 0;i < reminders.size(); i++){
        LPReminderInfo r = reminders.at(i);
        wchar_t header[16];
        wsprintf(header,L"Reminder%d",i+1);
        IniWriteInt(header,L"LAC",r->LAC,ini_reminder);
        IniWriteInt(header,L"CID",r->CID,ini_reminder);
        IniWriteInt(header,L"isEnable",r->isEna,ini_reminder);
        IniWriteInt(header,L"RepeatType",r->type,ini_reminder);
        IniWriteString(header,L"EventText",r->text,ini_reminder);
        IniWriteString(header,L"LocalName",r->name,ini_reminder);
        IniWriteString(header,L"NextAlarm",r->nextAlarm,ini_reminder);
    }
}

void GeoReminder::updateReminderList(LPReminderInfo r){
    int nIndex = getReminderIndex(r->LAC,r->CID);
    if(nIndex < 0) return;
    wchar_t header[16];
    wsprintf(header,L"Reminder%d",nIndex+1);
    IniWriteInt(header,L"LAC",r->LAC,ini_reminder);
    IniWriteInt(header,L"CID",r->CID,ini_reminder);
    IniWriteInt(header,L"isEnable",r->isEna,ini_reminder);
    IniWriteInt(header,L"RepeatType",r->type,ini_reminder);
    IniWriteString(header,L"EventText",r->text,ini_reminder);
    IniWriteString(header,L"LocalName",r->name,ini_reminder);
    IniWriteString(header,L"NextAlarm",r->nextAlarm,ini_reminder);
}

bool GeoReminder::addReminder(LPReminderInfo r){
    bool ret = false;;
    if(r->CID != 0 && r->LAC != 0){
        if(!updateReminder(r)){
            LPReminderInfo pr = new ReminderInfo_t;
            pr->LAC = r->LAC;
            pr->CID = r->CID;
            pr->type = r->type;
            C::newstrcpy(&pr->text, r->text);
            C::newstrcpy(&pr->name, r->name);
            C::newstrcpy(&pr->nextAlarm, r->nextAlarm);
            reminders.push_back(pr);
            saveReminderList();
            ret = true;
        }
    }
    return ret;
}

bool GeoReminder::updateReminder(LPReminderInfo r){
    bool ret = false;
    if(r->CID != 0 && r->LAC != 0){
        LPReminderInfo pr = getReminder(r->LAC, r->CID);
        if(pr){
            if(pr != r){
                pr->LAC = r->LAC;
                pr->CID = r->CID;
                pr->type = r->type;
                C::newstrcpy(&pr->text, r->text);
                C::newstrcpy(&pr->name, r->name);
                C::newstrcpy(&pr->nextAlarm, r->nextAlarm);
            }
            updateReminderList(pr);
            ret = true;
        }
    }
    return ret;
}

HANDLE GeoReminder::setNextCheckInterval(int interval){
    if(interval == 0) return 0;

    SYSTEMTIME now;
    SYSTEMTIME nextPoint;
    GetLocalTime(&now);
    nextPoint = now;
    interval += 1;
    nextPoint.wSecond += interval;
    if(nextPoint.wSecond > 59){
        int m = nextPoint.wSecond / 60;
        nextPoint.wSecond %= 60;
        nextPoint.wMinute += m;
        if(nextPoint.wMinute >59){
            int h = nextPoint.wMinute / 60;
            nextPoint.wMinute %= 60;
            nextPoint.wHour += h;
            if(nextPoint.wHour > 23){
                int d = nextPoint.wHour / 24;
                nextPoint.wHour %= 24;
                for(int i = 0; i < d; i++){
                    DateTime::OneDayDate(nextPoint);
                }
            }
        }
    }

    HANDLE hReminder = Notify(nextPoint);
    IniWriteInt(L"Config",L"ReminderID",(DWORD)hReminder,ini_path);
    return hReminder;
}

LPReminderInfo GeoReminder::deleteReminder(LPReminderInfo r){
    LPReminderInfo r_evnt = getReminder(r->LAC,r->CID);

    if(r_evnt){
        r_evnt->isRemoved = true;
    }
    return r_evnt;
}

void GeoReminder::clearReminder(){
    for(int i = 0; i < reminders.size(); i++){
        LPReminderInfo r = reminders.at(i);
        delete r;
    }
    reminders.clear();
}

LPReminderInfo GeoReminder::getReminder(int lac, int cid){
    for(int i = 0; i < reminders.size(); i++){
        LPReminderInfo r = reminders.at(i);
        if(r->LAC == lac && r->CID == cid){
            return r;
        }
    }
    return 0;
}

LPReminderInfo GeoReminder::getReminderByIndex(int idx){
    if(idx >= reminders.size()) return 0;
    return reminders.at(idx);
}

int GeoReminder::getReminderIndex(int lac, int cid){
    if(reminders.size() == 0) return -1;

    for(int i = 0; i < reminders.size(); i++){
        LPReminderInfo r = reminders.at(i);
        if(r->LAC == lac && r->CID == cid){
            return i;
        }
    }
    return -1;
}

//TODO: check current reminder
LPReminderInfo GeoReminder::checkCurrentReminder(){
    LPReminderInfo preminder;
    return preminder;
}

LPReminderInfo GeoReminder::checkReminder(int lac, int cid){
    LPReminderInfo preminder = getReminder(lac,cid);
    if(preminder){
        if(preminder->isEna){
            SYSTEMTIME alarmTime;
            swscanf(preminder->nextAlarm,
                L"%04d%02d%02d%02d%02d%02d",
                &alarmTime.wYear,&alarmTime.wMonth,&alarmTime.wDay,
                &alarmTime.wHour,&alarmTime.wMinute,&alarmTime.wSecond);
            preminder->isEna = false;
            saveReminderList();
        }else{
            preminder = 0;
        }
    }
    return preminder;
}

//////

Ui_GeoReminderWnd::Ui_GeoReminderWnd(void)
{
    r = 0;
}

Ui_GeoReminderWnd::~Ui_GeoReminderWnd(void)
{
}

void Ui_GeoReminderWnd::updateUi(){
    wchar_t s[16];
    if(m_EdtLAC.GetText().Length() == 0){
        wsprintf(s, L"%d", r->LAC);
        m_EdtLAC.SetText(s);
    }
    if(m_EdtCI.GetText().Length() == 0){
        wsprintf(s, L"%02d", r->CID);
        m_EdtCI.SetText(s);
    }

    if(m_EdtLACName.GetText().Length() == 0){
        m_EdtLACName.SetText(r->name);
    }
    if(m_EdtNote.GetText().Length() == 0){
        m_EdtNote.SetText(r->text);
    }
#if 0
    if(m_BtnReminderSW.GetState()){
        m_BtnReminder.SetText2(L"启用");
    }else{
        m_BtnReminder.SetText2(L"禁用");
    }
    m_BtnReminder.Invalidate();
    m_BtnReminder.Update();
    m_BtnRepeat.Invalidate();
    m_BtnRepeat.Update();
    m_BtnRepeatSW.Invalidate();
    m_BtnRepeatSW.Update();
#endif
    m_EdtLAC.Invalidate();
    m_EdtLAC.Update();
    m_EdtCI.Invalidate();
    m_EdtCI.Update();
    m_EdtLACName.Invalidate();
    m_EdtLACName.Update();
    m_EdtNote.Invalidate();
    m_EdtNote.Update();
}


BOOL Ui_GeoReminderWnd::OnInitDialog() {
    // Must all the Init of parent class first!
    if (!CMzWndEx::OnInitDialog()) {
        return FALSE;
    }

    // Then init the controls & other things in the window
    int y = 0;
    m_ScrollWin.SetPos(0, y, GetWidth(), GetHeight() - MZM_HEIGHT_TEXT_TOOLBAR);
    m_ScrollWin.EnableScrollBarV(true);
    AddUiWin(&m_ScrollWin);

#if 0
    m_BtnReminder.SetPos(0,y, GetWidth() - 120, MZM_HEIGHT_BUTTONEX);
    m_BtnReminder.SetText(L"地理提醒");
    m_BtnReminder.SetButtonType(MZC_BUTTON_LINE_NONE);
    m_BtnReminder.SetEnable(false);
    m_ScrollWin.AddChild(&m_BtnReminder);
    m_BtnReminderSW.SetPos(GetWidth() - 120,y, 120, MZM_HEIGHT_BUTTONEX);
    m_BtnReminderSW.SetButtonType(MZC_BUTTON_SWITCH);
    m_BtnReminderSW.SetButtonMode(MZC_BUTTON_MODE_HOLD);
    m_BtnReminderSW.SetID(MZ_IDC_BTN_REMINDER);
    m_ScrollWin.AddChild(&m_BtnReminderSW);

    y+=MZM_HEIGHT_BUTTONEX;
#endif
    int x = 0;
    m_EdtLAC.SetPos(x, y, GetWidth()/2, MZM_HEIGHT_SINGLELINE_EDIT);
    m_EdtLAC.SetSipMode(IM_SIP_MODE_DIGIT, MZM_HEIGHT_TEXT_TOOLBAR);
    m_EdtLAC.SetTip2(L"小区");
    m_EdtLAC.SetLeftInvalid(100);
    m_ScrollWin.AddChild(&m_EdtLAC);

    x += GetWidth()/2;
    m_EdtCI.SetPos(x, y, GetWidth()/2, MZM_HEIGHT_SINGLELINE_EDIT);
    m_EdtCI.SetSipMode(IM_SIP_MODE_DIGIT, MZM_HEIGHT_TEXT_TOOLBAR);
    m_EdtCI.SetTip2(L"基站");
    m_EdtCI.SetLeftInvalid(100);
    m_ScrollWin.AddChild(&m_EdtCI);

    y+=MZM_HEIGHT_SINGLELINE_EDIT;
    m_EdtLACName.SetPos(0, y, GetWidth(), MZM_HEIGHT_SINGLELINE_EDIT);
    m_EdtLACName.SetSipMode(IM_SIP_MODE_GEL_PY, MZM_HEIGHT_TEXT_TOOLBAR);
    m_EdtLACName.SetTip2(L"地点名称");
    m_EdtLACName.SetLeftInvalid(150);
    m_ScrollWin.AddChild(&m_EdtLACName);

    y+=MZM_HEIGHT_SINGLELINE_EDIT;
#if 0
    m_BtnRepeat.SetPos(0,y,GetWidth() - 120,MZM_HEIGHT_BUTTONEX);
    m_BtnRepeat.SetButtonType(MZC_BUTTON_LINE_NONE);
    m_BtnRepeat.SetText(L"循环提醒");
    m_BtnRepeat.SetEnable(false);
    m_ScrollWin.AddChild(&m_BtnRepeat);

    m_BtnRepeatSW.SetPos(GetWidth() - 120,y, 120, MZM_HEIGHT_BUTTONEX);
    m_BtnRepeatSW.SetButtonType(MZC_BUTTON_SWITCH);
    m_BtnRepeatSW.SetButtonMode(MZC_BUTTON_MODE_HOLD);
    m_BtnRepeatSW.SetID(MZ_IDC_BTN_REPEAT);
    m_ScrollWin.AddChild(&m_BtnRepeatSW);

    y+=MZM_HEIGHT_BUTTONEX;
#endif
    m_EdtNote.SetPos(0, y, GetWidth(), GetHeight() - y - MZM_HEIGHT_TEXT_TOOLBAR);
    m_EdtNote.SetSipMode(IM_SIP_MODE_GEL_PY);
    m_EdtNote.SetEditBgType(UI_EDIT_BGTYPE_FILL_WHITE_AND_TOPSHADOW);
    m_ScrollWin.AddChild(&m_EdtNote);

    m_Toolbar.SetPos(0, GetHeight() - MZM_HEIGHT_TEXT_TOOLBAR, GetWidth(), MZM_HEIGHT_TEXT_TOOLBAR);
    m_Toolbar.SetButton(0, true, true, L"取消");
    m_Toolbar.SetButton(1, true, true, L"删除");
    m_Toolbar.SetButton(2, true, true, L"确定");
    m_Toolbar.SetID(MZ_IDC_TOOLBAR_REMINDER);
    AddUiWin(&m_Toolbar);
    updateUi();
    return TRUE;
}

void Ui_GeoReminderWnd::OnMzCommand(WPARAM wParam, LPARAM lParam) {
    UINT_PTR id = LOWORD(wParam);
    switch (id) {
        case MZ_IDC_BTN_REMINDER:
            {
                updateUi();
                break;
            }
        case MZ_IDC_BTN_REPEAT:
            {
#if 0
                if(m_BtnRepeatSW.GetState() == MZCS_BUTTON_NORMAL){
                    r->type = REPEAT_1TIME;
                }else{
                    r->type = REPEAT_DAY;
                }
                m_BtnRepeat.SetFocus(true);
                updateUi();
#endif
                break;
            }
        case MZ_IDC_TOOLBAR_REMINDER:
            {
                int nIndex = lParam;
                if(nIndex == 1){
                    if(MzMessageBoxEx(m_hWnd,L"确实要删除此记录？",L"确认",MZ_YESNO,false) == 1){
                        georeminder.deleteReminder(r);
                    }
                    EndModal(ID_OK);
                    return;
                }
                if(nIndex == 2){	//确定
                    //更新
#if 0
                    if(m_BtnReminderSW.GetState() != MZCS_BUTTON_PRESSED){
                        r->type = REPEAT_NONE;
                    }
#endif
                    if(m_EdtLACName.GetText().IsEmpty() ||
                        m_EdtLAC.GetText().IsEmpty() ||
                        m_EdtCI.GetText().IsEmpty())
                    {
                        return;
                    }
                    r->name = m_EdtLACName.GetText();
                    r->text = m_EdtNote.GetText();
                    r->LAC = _wtoi(m_EdtLAC.GetText().C_Str());
                    r->CID = _wtoi(m_EdtCI.GetText().C_Str());
                    EndModal(ID_OK);
                    return;
                }
                if(nIndex == 0){	//取消
                    EndModal(ID_CANCEL);
                    return;
                }
            }
    }
}

LRESULT Ui_GeoReminderWnd::MzDefWndProc(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case MZ_WM_UI_REMINDER_DLG_OFF:
            {
                EndModal(ID_CANCEL);
                return 0;
            }
        case MZ_WM_MOUSE_NOTIFY:
            {
                int nID = LOWORD(wParam);
                int nNotify = HIWORD(wParam);
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
            }
    }
    return CMzWndEx::MzDefWndProc(message, wParam, lParam);
}
