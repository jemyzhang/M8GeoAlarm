#ifndef _UI_MAIN_H
#define _UI_MAIN_H

// include the MZFC library header file
#include <mzfc_inc.h>
#include "ui_reminder.h"
#define MZ_WM_UI_REMINDER_UPDATE					MZFC_WM_MESSAGE+0x0100
#define MZ_WM_UI_REMINDER_SETNEW					MZ_WM_UI_REMINDER_UPDATE+1
#define MZ_WM_UI_REMINDER_EDITOR_ON					MZ_WM_UI_REMINDER_UPDATE+2
#define MZ_WM_UI_REMINDER_EDITOR_OFF				MZ_WM_UI_REMINDER_UPDATE+3
#define MZ_WM_UI_REMINDER_REFRESH					MZ_WM_UI_REMINDER_UPDATE+4
#define MZ_WM_UI_REMINDER_DLG_OFF					MZ_WM_UI_REMINDER_UPDATE+5

class ReminderList : public UiList {
public:
    // override the DrawItem member function to do your own drawing of the list
	ReminderList() {  }
	void setItemCount(int count){
		RemoveAll();
		for(int i = 0; i < count; i++){
			AddItem(ListItem());
		}
	}
	void DrawItem(HDC hdcDst, int nIndex, RECT* prcItem, RECT *prcWin, RECT *prcUpdate);
};

class UiImage : public UiWin
{
public:
	UiImage(void){
		_reqUpdate = true;
		pimg = 0;
	}
	~UiImage(void){
	}
	void setupImage(ImagingHelper* img) {
		pimg = img;
	}
	virtual void PaintWin(HDC hdcDst, RECT* prcWin, RECT* prcUpdate);
	virtual void Update() {
		_reqUpdate = true;
		UiWin::Update();
	}
private:
	bool _reqUpdate;
	ImagingHelper *pimg;
};

class UiEditor : public UiSingleLineEdit{
public:
	virtual void OnFocusd(UiWin* pWinPrev){
		PostMessage(MZ_WM_UI_REMINDER_EDITOR_ON,NULL,NULL);
		MzOpenSip();
		//UiSingleLineEdit::OnFocusd(pWinPrev);
	}
	virtual void OnLostFocus(UiWin* pWinNext){
		PostMessage(MZ_WM_UI_REMINDER_EDITOR_OFF,NULL,NULL);
		MzCloseSip();
		//UiSingleLineEdit::OnLostFocus(pWinNext);
	}

};

class UiLineEdit : public UiButton{
public:
	UiLineEdit();
public:
	void SetTip(LPCTSTR text) { m_Edit.SetTip(text); }
	virtual void SetText(LPCTSTR text) { m_Edit.SetText(text); }
	virtual CMzString& GetText() { return m_Edit.GetText(); }
	void SetStatus(bool en) { 
		if(en){
			m_Edit.SetEnable(true);
			m_Edit.SetFocus(true);
		}else{
			m_Edit.SetEnable(false);
			m_Edit.SetFocus(false);
		}
		m_Edit.Invalidate();
		m_Edit.Update();
	}
	virtual void SetPos(int x, int y, int w, int h, UINT flags=0);
protected:
	virtual int OnLButtonUp(UINT fwKeys, int xPos, int yPos){
		if(m_Edit.IsEnable()){
			m_Edit.SetEnable(false);
			m_Edit.SetFocus(false);
		}else{
			m_Edit.SetEnable(true);
			m_Edit.SetFocus(true);
		}
		m_Edit.Invalidate();
		m_Edit.Update();
		return UiButton::OnLButtonUp(fwKeys,xPos,yPos);
	}
private:
	UiEditor m_Edit;
	UiImage m_Image;
};

// Main window derived from CMzWndEx

class Ui_MainWnd : public CMzWndEx {
    MZ_DECLARE_DYNAMIC(Ui_MainWnd);
public:
	Ui_MainWnd();
	~Ui_MainWnd();
	void setupDUi(bool = false);	//dynamic ui
	void refreshReminderList(bool updatelist = false);
	void refreshSignalImage();
	//void CALLBACK animateSignalImage(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime);
	void OnTimer(UINT nIDEvent);
public:
    UiScrollWin m_ScrollWin;
    UiToolbar_Text m_Toolbar;

	UiCaption m_lblTitle;

	UiSingleLineEdit m_lblLACI;
	UiSingleLineEdit m_lblCI;
	UiButtonEx m_BtnEnaReminder;
	UiButtonEx m_BtnEnaReminderSW;
	UiLineEdit m_EdtLocName;	//local name
	UiButton m_BtnAddModifyLocName;
	ReminderList m_ReminderList;

	UiImage m_ImageSignal;
protected:
    // Initialization of the window (dialog)
    virtual BOOL OnInitDialog();

    // override the MZFC window messages handler
    LRESULT MzDefWndProc(UINT message, WPARAM wParam, LPARAM lParam);

    // override the MZFC command handler
    virtual void OnMzCommand(WPARAM wParam, LPARAM lParam);
private:
	HANDLE hReminder;
	int interval;
	bool bprohibit;
	int edTimeout;	//±à¼­Ê±³¬Ê±
	HWND editDlgHwnd;	//±à¼­½çÃæ×´Ì¬
};


#endif /*_UI_MAIN_H*/