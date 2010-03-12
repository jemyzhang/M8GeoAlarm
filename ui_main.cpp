#include "ui_main.h"
#include "M8GeoAlarm.h"

#include "ReadWriteIni.h"

#include "resource.h"

#include "cMzCommon.h"

#include "ui_about.h"
using namespace cMzCommon;
#include <notify.h>

#include "cellid_info.h"

#define MZ_IDC_TOOLBAR_MAIN 101
#define MZ_IDC_SCROLLWIN 102

#define MZ_IDC_BUTTON_DATE 103
#define MZ_IDC_BUTTON_ACCOUNT 104
#define MZ_IDC_BUTTON_CATEGORY 105
#define MZ_IDC_EDIT_YUAN 106
#define MZ_IDC_EDIT_NOTE 107
#define MZ_IDC_BUTTON_ADD 108
#define MZ_IDC_BUTTON_TOACCOUNT 109
#define MZ_IDC_CAPTION_TITLE 110
#define MZ_IDC_BUTTON_REMINDER 111
#define MZ_IDC_BUTTON_CALCULATOR 112


#define MZ_IDC_BTN_ENAREMINDER	120
#define MZ_IDC_BTN_ADDLOC		121
#define MZ_IDC_EDT_LOCNAME		122
#define MZ_IDC_LIST				123

void UiImage::PaintWin(HDC hdcDst, RECT* prcWin, RECT* prcUpdate){
	UiWin::PaintWin(hdcDst,prcWin,prcUpdate);
//	if(_reqUpdate){
//		_reqUpdate = false;
		if(pimg){
			pimg->Draw(hdcDst,prcWin,true,true);
		}
//	}
}

UiLineEdit::UiLineEdit(){
	m_Edit.SetEnable(false);
	m_Edit.SetSipMode(IM_SIP_MODE_GEL_PY,MZM_HEIGHT_TEXT_TOOLBAR);
	AddChild(&m_Edit);
	m_Image.SetEnable(false);
	AddChild(&m_Image);
	SetButtonType(MZC_BUTTON_NONE);
}

void UiLineEdit::SetPos(int x, int y, int w, int h, UINT flags){
	UiButton::SetPos(x,y,w,h,flags);
	m_Edit.SetPos(0,0,GetWidth()-30,GetHeight());
	m_Edit.Invalidate();
	m_Edit.Update();
	m_Image.setupImage(pimg[IDB_PNG_KEYPAD - IDB_PNG_BEGIN]);
	m_Image.SetPos(GetWidth() - 65, GetHeight()/2 - 16, 32,32);
	m_Image.Invalidate();
	m_Image.Update();
}

void ReminderList::DrawItem(HDC hdcDst, int nIndex, RECT* prcItem, RECT *prcWin, RECT *prcUpdate) {
	// draw the high-light background for the selected item
    if (nIndex == GetSelectedIndex()) {
        MzDrawSelectedBg(hdcDst, prcItem);
    }
	//rect1
	RECT rectLine10 = {
		prcItem->left,		prcItem->top,
		prcItem->right - 100 - 230,	prcItem->top + (prcItem->bottom - prcItem->top)/2
	};
	RECT rectLine11 = {
		rectLine10.right,		prcItem->top,
		rectLine10.right + 230,	prcItem->top + (prcItem->bottom - prcItem->top)/2
	};
	RECT rectLine2 = {
		prcItem->left,		rectLine11.bottom,
		rectLine11.right,	prcItem->bottom
	};
	wchar_t wbuf[32];
	LPReminderInfo preminder = georeminder.getReminderByIndex(nIndex);
	wsprintf(wbuf,L"(%d,%d)",preminder->LAC,preminder->CID);

	::SetTextColor( hdcDst , RGB(41,136,226) );
	HFONT hf = FontHelper::GetFont( 20 );
    SelectObject( hdcDst , hf );
	MzDrawText( hdcDst , wbuf, &rectLine10 , DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS );
	DeleteObject(hf);

	::SetTextColor( hdcDst , RGB(0,0,0) );
	hf = FontHelper::GetFont( 25 );
    SelectObject( hdcDst , hf );
	MzDrawText( hdcDst , preminder->name, &rectLine11 , DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS );
	DeleteObject(hf);

	::SetTextColor( hdcDst , RGB(57,168,255) );
	hf = FontHelper::GetFont( 20 );
    SelectObject( hdcDst , hf );
	MzDrawText( hdcDst , preminder->text, &rectLine2 , DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS );
	DeleteObject(hf);
	RECT rectImage1 = {
		rectLine2.right,		prcItem->top + (prcItem->bottom - prcItem->top - 48)/2,
		rectLine2.right + 48,	prcItem->top + (prcItem->bottom - prcItem->top)/2 + 24
	};
	int imgID = preminder->isEna ? IDB_PNG_ALARM_EN - IDB_PNG_BEGIN : IDB_PNG_ALARM_DIS - IDB_PNG_BEGIN;
	pimg[imgID]->Draw(hdcDst,&rectImage1);

	int iw = imgArrow->GetImageWidth();
	int ih = imgArrow->GetImageHeight();
	RECT rectImage2 = {
		prcItem->right - iw - 5,		prcItem->top + (prcItem->bottom - prcItem->top - ih)/2,
		prcItem->right - 5,	prcItem->top + (prcItem->bottom - prcItem->top)/2 + ih/2
	};
	imgArrow->Draw(hdcDst,&rectImage2);
}

MZ_IMPLEMENT_DYNAMIC(Ui_MainWnd)

Ui_MainWnd::Ui_MainWnd(){
	interval = 10;
	//setup path
	wchar_t currpath[128];
	if(File::GetCurrentPath(currpath)){
		wsprintf(ini_path,L"%s\\M8GeoAlarm.ini",currpath);
	}else{
		wsprintf(ini_path,CONFIG_INI);
	}
	if(!IniReadInt(L"Config",L"ReminderID",(DWORD*)&hReminder,ini_path)){
		hReminder = 0;
		IniCreateFile(ini_path);
		IniWriteInt(L"Config",L"ReminderID",(DWORD)hReminder,ini_path);
	}
	bprohibit = false;
	edTimeout = 0;
	editDlgHwnd = 0;
}

Ui_MainWnd::~Ui_MainWnd(){
	KillTimer(m_hWnd,0x1001);
}

BOOL Ui_MainWnd::OnInitDialog() {
    // Must all the Init of parent class first!
    if (!CMzWndEx::OnInitDialog()) {
        return FALSE;
    }

    // Then init the controls & other things in the window
    int y = 0;
	wchar_t name[128];
	wsprintf(name,L"%s v%s",APPNAME,VER_STRING);
	m_lblTitle.SetPos(0,y,GetWidth(),MZM_HEIGHT_CAPTION/2);
	m_lblTitle.SetText(name);
	m_lblTitle.SetTextSize(m_lblTitle.GetTextSize()/2);
	m_lblTitle.SetTextWeight(FW_BOLD);
	m_lblTitle.SetID(MZ_IDC_CAPTION_TITLE);
	m_lblTitle.EnableNotifyMessage(true);
    AddUiWin(&m_lblTitle);

	y+=MZM_HEIGHT_CAPTION/2;
	m_ImageSignal.SetPos(7,y + (MZM_HEIGHT_BUTTONEX - 48)/2, 48, 48);
    AddUiWin(&m_ImageSignal);

	m_BtnEnaReminder.SetPos(55,y, GetWidth() - 175, MZM_HEIGHT_BUTTONEX);
	m_BtnEnaReminder.SetText(L"基站信息");
	m_BtnEnaReminder.SetTextMaxLen(0);
    m_BtnEnaReminder.SetButtonType(MZC_BUTTON_LINE_NONE);
	m_BtnEnaReminder.SetEnable(false);
    AddUiWin(&m_BtnEnaReminder);

	m_BtnEnaReminderSW.SetPos(GetWidth() - 120,y, 120, MZM_HEIGHT_BUTTONEX);
    m_BtnEnaReminderSW.SetButtonType(MZC_BUTTON_SWITCH);
	m_BtnEnaReminderSW.SetButtonMode(MZC_BUTTON_MODE_HOLD);
	m_BtnEnaReminderSW.SetID(MZ_IDC_BTN_ENAREMINDER);
    AddUiWin(&m_BtnEnaReminderSW);

	y+=MZM_HEIGHT_BUTTONEX;
	m_lblLACI.SetPos(0, y, GetWidth()/2, MZM_HEIGHT_SINGLELINE_EDIT);
	m_lblLACI.SetTip2(L"小区");
    m_lblLACI.SetLeftInvalid(100);
	m_lblLACI.SetEnable(false);
    AddUiWin(&m_lblLACI);

	m_lblCI.SetPos(GetWidth()/2, y, GetWidth()/2, MZM_HEIGHT_SINGLELINE_EDIT);
    m_lblCI.SetTip2(L"基站");
    m_lblCI.SetLeftInvalid(100);
	m_lblCI.SetEnable(false);
    AddUiWin(&m_lblCI);

 	y+=MZM_HEIGHT_SINGLELINE_EDIT;
	m_EdtLocName.SetPos(0, y, GetWidth() - 150, 50);
	m_EdtLocName.SetID(MZ_IDC_EDT_LOCNAME);
	m_EdtLocName.SetEnable(false);
    AddUiWin(&m_EdtLocName);

	m_BtnAddModifyLocName.SetPos(GetWidth() - 150, y, 150, 50);
	m_BtnAddModifyLocName.SetText(L"新增");	//新增，修改，完成
	m_BtnAddModifyLocName.SetTextColor(RGB(64,64,64));
	m_BtnAddModifyLocName.SetButtonType(MZC_BUTTON_DOWNLOAD);
	m_BtnAddModifyLocName.SetID(MZ_IDC_BTN_ADDLOC);
    AddUiWin(&m_BtnAddModifyLocName);

 	y+=50;
	m_ReminderList.SetPos(0, y, GetWidth(), GetHeight() - MZM_HEIGHT_TEXT_TOOLBAR - y);
	m_ReminderList.SetID(MZ_IDC_LIST);
	m_ReminderList.EnableNotifyMessage(true);
    AddUiWin(&m_ReminderList);

	m_Toolbar.SetPos(0, GetHeight() - MZM_HEIGHT_TEXT_TOOLBAR, GetWidth(), MZM_HEIGHT_TEXT_TOOLBAR);
    m_Toolbar.SetID(MZ_IDC_TOOLBAR_MAIN);
	m_Toolbar.SetButton(0, true, true, L"关于");
//	m_Toolbar.SetButton(1, true, true, L"设置");
	m_Toolbar.SetButton(2, true, true, L"退出");
    AddUiWin(&m_Toolbar);
	if(hReminder == 0){
		m_BtnEnaReminderSW.SetState(MZCS_BUTTON_NORMAL);
	}else{
		DWORD dwBytes;
		BYTE bytes[10];
		bool ret = CeGetUserNotification(hReminder,10,&dwBytes,(LPBYTE)&bytes);
		if(dwBytes == 0){
			m_BtnEnaReminderSW.SetState(MZCS_BUTTON_NORMAL);
			m_BtnEnaReminder.SetText2(L"关闭");
		}else{
			m_BtnEnaReminderSW.SetState(MZCS_BUTTON_PRESSED);
			m_BtnEnaReminder.SetText2(L"启用");
			CeClearUserNotification(hReminder);
			hReminder = georeminder.setNextCheckInterval(1);
		}
	}
	refreshSignalImage();
	refreshReminderList(true);
	SetTimer(m_hWnd,0x1001,500,NULL);

    return TRUE;
}

void Ui_MainWnd::setupDUi(bool update){
	DWORD lac, cid;
	if(!bprohibit && GetCurrentCellInfo(0,0,&lac,&cid)){
		if(lac == 0 || cid == 0) return;
		LPReminderInfo preminder = georeminder.getReminder(lac,cid);
		if(preminder){
			m_EdtLocName.SetText(preminder->name);
			m_BtnAddModifyLocName.SetText(L"修改");
			m_BtnAddModifyLocName.SetEnable(true);
			int nIndex = georeminder.getReminderIndex(lac,cid);
#if 0
			if(!m_ReminderList.IsMouseDownAtScrolling()){
				m_ReminderList.ScrollTo(UI_SCROLLTO_POS,nIndex);
			}
#endif
			m_ReminderList.SetSelectedIndex(nIndex);
			refreshReminderList();
		}else{
			m_EdtLocName.SetText(L"\0");
			m_EdtLocName.SetTip(L"无记录");
			m_BtnAddModifyLocName.SetText(L"新增");
			m_BtnAddModifyLocName.SetEnable(true);
#if 0
			if(!m_ReminderList.IsMouseDownAtScrolling()){
				m_ReminderList.ScrollTo();
			}
#endif
			m_ReminderList.SetSelectedIndex(-1);
			refreshReminderList();
		}
		m_EdtLocName.SetEnable(true);
		m_EdtLocName.Invalidate();
		m_EdtLocName.Update();
		m_BtnAddModifyLocName.Invalidate();
		m_BtnAddModifyLocName.Update();
		wchar_t buf[16];
		wsprintf(buf,L"%d",lac);
		m_lblLACI.SetText(buf);
		m_lblLACI.Invalidate();
		m_lblLACI.Update();
		wsprintf(buf,L"%d",cid);
		m_lblCI.SetText(buf);
		m_lblCI.Invalidate();
		m_lblCI.Update();
	}
	//检查编辑状态超时
	//检查键盘错误
	if(bprohibit){
		edTimeout--;
		if(editDlgHwnd){
			if(edTimeout == 0){
				::PostMessage(editDlgHwnd,MZ_WM_UI_REMINDER_DLG_OFF,NULL,NULL);
				//PostMessage(MZ_WM_UI_REMINDER_DLG_OFF,NULL,NULL);
			}
		}else{
			if(edTimeout == 0 || !MzIsSipOpen()){
				m_EdtLocName.SetStatus(false);
				m_EdtLocName.Invalidate();
				m_EdtLocName.Update();
			}
		}
	}
}

void Ui_MainWnd::refreshReminderList(bool updatelist){
	if(updatelist){
		int size = georeminder.reminders.size();
		m_ReminderList.setItemCount(size);
	}
	m_ReminderList.Invalidate();
	m_ReminderList.Update();
}

void Ui_MainWnd::refreshSignalImage(){
	if(m_BtnEnaReminderSW.GetState() == MZCS_BUTTON_PRESSED){
		m_ImageSignal.setupImage(pimg[IDB_PNG_SIGNAL_0 - IDB_PNG_BEGIN]);
	}else{
		m_ImageSignal.setupImage(pimg[IDB_PNG_SIGNAL_NO - IDB_PNG_BEGIN]);
	}
	m_ImageSignal.Invalidate();
	m_ImageSignal.Update();
}

//void Ui_MainWnd::animateSignalImage(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime){
void Ui_MainWnd::OnTimer(UINT nIDEvent){
	static int cnt = 0;
	switch(nIDEvent){
		case 0x1001:
			if(m_BtnEnaReminderSW.GetState() == MZCS_BUTTON_PRESSED){
				m_ImageSignal.setupImage(pimg[IDB_PNG_SIGNAL_0 - IDB_PNG_BEGIN + cnt]);
				m_ImageSignal.Invalidate();
				m_ImageSignal.Update();
				cnt ++;
				cnt = cnt > 2 ? 0 : cnt;
			}
			break;
		case 0x1002:
			if(m_BtnEnaReminderSW.GetState() == MZCS_BUTTON_PRESSED){
				PostMessage(MZ_WM_UI_REMINDER_UPDATE,NULL,NULL);
			}
			break;
	}
}

LRESULT Ui_MainWnd::MzDefWndProc(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
		case MZ_WM_UI_REMINDER_UPDATE:
		{
			setupDUi(true);
			return 0;
		}
		case MZ_WM_UI_REMINDER_SETNEW:
		{
			if(m_BtnEnaReminderSW.GetState() == MZCS_BUTTON_PRESSED && !bprohibit){
				hReminder = georeminder.setNextCheckInterval(interval);
			}
			return 0;
		}
		case MZ_WM_UI_REMINDER_EDITOR_ON:	//editor enabled
		{
			if(m_BtnEnaReminderSW.GetState() == MZCS_BUTTON_PRESSED && bprohibit == false){
				CeClearUserNotification(hReminder);
				bprohibit = true;
				edTimeout = 30;	//1min
			}
			return 0;
		}
		case MZ_WM_UI_REMINDER_EDITOR_OFF:
		{
			if(m_BtnEnaReminderSW.GetState() == MZCS_BUTTON_PRESSED && bprohibit == true){
				bprohibit = false;
				hReminder = georeminder.setNextCheckInterval(interval);
			}
			return 0;
		}
		case MZ_WM_UI_REMINDER_REFRESH:	//refresh list after alarm
		{
			georeminder.loadReminderList();
			refreshReminderList();
			return 0;
		}
        case MZ_WM_MOUSE_NOTIFY:
        {
            int nID = LOWORD(wParam);
            int nNotify = HIWORD(wParam);
            int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			if(nID == MZ_IDC_LIST && nNotify == MZ_MN_LBUTTONUP){
				if (!m_ReminderList.IsMouseDownAtScrolling() && !m_ReminderList.IsMouseMoved()) {
					int nIndex = m_ReminderList.CalcIndexOfPos(x, y);
					if(nIndex != -1){
						LPReminderInfo r = georeminder.getReminderByIndex(nIndex);
						int pos = m_ReminderList.GetWidth() - imgArrow->GetImageWidth() - 5;
						if(x > pos){
							Ui_GeoReminderWnd dlg;
							RECT rcWork = MzGetWorkArea();
							dlg.setupReminderInfo(r);
							if(rcWork.top == 0) rcWork.top = 40;
							dlg.Create(rcWork.left, rcWork.top, RECT_WIDTH(rcWork), RECT_HEIGHT(rcWork),
									m_hWnd, 0, WS_POPUP);
							// set the animation of the window
							dlg.SetAnimateType_Show(MZ_ANIMTYPE_SCROLL_BOTTOM_TO_TOP_2);
							dlg.SetAnimateType_Hide(MZ_ANIMTYPE_SCROLL_TOP_TO_BOTTOM_1);
							editDlgHwnd = dlg.m_hWnd;
							PostMessage(MZ_WM_UI_REMINDER_EDITOR_ON,NULL,NULL);
							int nRet = dlg.DoModal();
							editDlgHwnd = 0;
							PostMessage(MZ_WM_UI_REMINDER_EDITOR_OFF,NULL,NULL);
							if(nRet == ID_OK){
								georeminder.saveReminderList();
								refreshReminderList(true);
							}
						}else
						if(x > pos - 48 && x < pos){
							if(r){
								r->isEna = ! r->isEna;
								georeminder.updateReminder(r);
								refreshReminderList();
							}
						}
					}
                }
                return 0;
            }
#if 0
            if (nID == MZ_IDC_LIST && nNotify == MZ_MN_MOUSEMOVE) {
                m_ReminderList.SetSelectedIndex(-1);
                m_ReminderList.Invalidate();
                m_ReminderList.Update();
                return 0;
            }
#endif
       }
	}
    return CMzWndEx::MzDefWndProc(message, wParam, lParam);
}


void Ui_MainWnd::OnMzCommand(WPARAM wParam, LPARAM lParam) {
    UINT_PTR id = LOWORD(wParam);
    switch (id) {
		case MZ_IDC_BTN_ENAREMINDER:
		{
			if(m_BtnEnaReminderSW.GetState() == MZCS_BUTTON_NORMAL){
				m_BtnEnaReminder.SetText2(L"关闭");
				CeClearUserNotification(hReminder);
				KillTimer(m_hWnd,0x1002);
				m_BtnAddModifyLocName.SetEnable(false);
				m_BtnAddModifyLocName.Invalidate();
				refreshSignalImage();
			}else{
				if(bprohibit) m_EdtLocName.SetStatus(false);
				m_BtnEnaReminder.SetText2(L"启用");
				m_BtnEnaReminder.Invalidate();
				hReminder = georeminder.setNextCheckInterval(1);
				SetTimer(m_hWnd,0x1002,1100,NULL);
			}
			m_BtnEnaReminder.Invalidate();
			m_BtnEnaReminder.Update();
			return;
		}
		case MZ_IDC_BTN_ADDLOC:
		{
			if(m_BtnEnaReminderSW.GetState() == MZCS_BUTTON_NORMAL){
				return;
			}
			if(m_EdtLocName.GetText().IsEmpty()){
				return;
			}
			ReminderInfo_t r;
			r.LAC = _wtoi(m_lblLACI.GetText().C_Str());
			r.CID = _wtoi(m_lblCI.GetText().C_Str());
			r.name = m_EdtLocName.GetText();
			LPReminderInfo pr = georeminder.getReminder(r.LAC, r.CID);
			if(pr){
				r.isEna = pr->isEna;
				r.text = pr->text;
				r.type = pr->type;
				r.nextAlarm = pr->nextAlarm;
				georeminder.updateReminder(&r);
				refreshReminderList();
			}else{
				georeminder.addReminder(&r);
				refreshReminderList(true);
			}
			m_EdtLocName.SetStatus(false);
		}
        case MZ_IDC_TOOLBAR_MAIN:
        {
            int nIndex = lParam;
			if(nIndex == 0){	//关于
				UI_AboutWnd dlg;
				RECT rcWork = MzGetWorkArea();
				dlg.Create(rcWork.left, rcWork.top, RECT_WIDTH(rcWork), RECT_HEIGHT(rcWork),
						m_hWnd, 0, WS_POPUP);
				// set the animation of the window
				dlg.SetAnimateType_Show(MZ_ANIMTYPE_SCROLL_RIGHT_TO_LEFT_2);
				dlg.SetAnimateType_Hide(MZ_ANIMTYPE_SCROLL_LEFT_TO_RIGHT_1);
				int nRet = dlg.DoModal();
                return;
			}
			if(nIndex == 2){	//确定
				PostQuitMessage(0);
				return;
			}
		}
	}
	CMzWndEx::OnMzCommand(wParam,lParam);
}

