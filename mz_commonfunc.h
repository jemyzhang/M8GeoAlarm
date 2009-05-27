#pragma once
// include the MZFC library header file
#include <mzfc_inc.h>
#include <list>

typedef enum TextEncode{
	ttcAnsi, 
	ttcUnicode, 
	ttcUnicodeBigEndian, 
	ttcUtf8
}TEXTENCODE_t;


class MZ_CommonC
{
public:
	static void newstrcpy(wchar_t** pdst,const wchar_t* src);
	//remove LF to "\n"
	static wchar_t* removeWrap(wchar_t* dst, wchar_t* src);
	//restore "\n" "\r" to LF
	static wchar_t* restoreWrap(wchar_t* dst, wchar_t* src);
	static wchar_t* _wcstok(wchar_t* _Str, const wchar_t *_Delim){
		static wchar_t* header;
		wchar_t* retval;
		if(_Str != NULL){
			header = _Str;
		}
		if(header == NULL || _Delim == NULL){
			return header;
		}
		int delimLen = lstrlen(_Delim);
		if(delimLen == 0){
			return header;
		}
		CMzString tmp = header;
		int cnt = 0;
		while(tmp.SubStr(cnt++,delimLen).Compare(_Delim) != 0){
			if(cnt >= tmp.Length()){
				retval = header;
				header = NULL;
				return retval;
			}
		}
		retval = header;
		header[cnt-1] = '\0';
		header += (cnt-1 + delimLen);
		return retval;
	}
};

class MZ_CommonDateTime
{
public:
	//获取当前日期
    //返回值:当前系统日期，格式：2009-12-11
	static void waitms(unsigned int ms){
		DWORD s = GetTickCount();
		DWORD e;
		do{	//wait
			//Do Events
			MSG msg;  
			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )  
			{  
				TranslateMessage(   &msg   );  
				DispatchMessage(   &msg   );  
			}
			e = GetTickCount();
		}while(e < s + ms);
	}

	static SYSTEMTIME StrToDate(wchar_t* datestr,wchar_t* format = L"%04d-%02d-%02d") {
		SYSTEMTIME sysDate;
		sysDate.wYear = 0;
		sysDate.wMonth = 0;
		sysDate.wDay = 0;
		if(datestr){
			swscanf(datestr,format,&sysDate.wYear,&sysDate.wMonth,&sysDate.wDay);
		}
		return sysDate;
	}
    static CMzString Date() {
        SYSTEMTIME sysTime;
        GetLocalTime(&sysTime);
        CMzString sDate(16);
        wsprintf(sDate.C_Str(), L"%04d-%02d-%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay);

        return sDate;
    }

    static void getDate(int *py = 0, int *pm = 0, int *pd = 0) {
        SYSTEMTIME sysTime;
        GetLocalTime(&sysTime);
		if(py) *py = sysTime.wYear;
		if(pm) *pm = sysTime.wMonth;
		if(pd) *pd = sysTime.wDay;
        return;
    }
	static void getPreDate(int &y, int &m) { 
		m--; 
		if(m <= 0){
			m = 12;
			y--;
		}
	}
	static void getNextDate(int &y, int &m) {
		m++; 
		if(m > 12){
			m = 1;
			y++;
		}
	}
	static int getWeekDay(int year,int month, int day);
	static bool isLeapyear(int year);
	static void OneDayDate(SYSTEMTIME &date,bool isYesterday = false);
    //获取当前时间
    //返回值:当前系统时间，格式：12:12:15

    static CMzString Time() {
        SYSTEMTIME sysTime;
        GetLocalTime(&sysTime);
        CMzString sTime(16);
        wsprintf(sTime.C_Str(), L"%02d:%02d:%02d", sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
        return sTime;
    }


    //获取当前时间
	static wchar_t* NowtoStr(){
		static wchar_t nowstr[128];
        SYSTEMTIME sysTime;
        GetLocalTime(&sysTime);
        wsprintf(nowstr, L"%04d%02d%02d%02d%02d%02d", 
			sysTime.wYear, sysTime.wMonth, sysTime.wDay,
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
        return nowstr;
	}

    static CMzString Now() {
        CMzString sDateTime(32), sDate(16), sTime(16);

        sDate = Date();
        sTime = Time();

        sDateTime = sDate + L" " + sTime;
        return sDateTime;
    }
	//获取一个月的天数
	static int getDays(int year, int month);
	static int checkDate(int year,int month, int day);
	//0: equal 1: large -1: small
	static int compareDate(SYSTEMTIME tm1, SYSTEMTIME tm2){
		if(tm1.wYear > tm2.wYear) return 1;
		if((tm1.wYear == tm2.wYear) &&
			(tm1.wMonth > tm2.wMonth)){
				return 1;
		}
		if((tm1.wYear == tm2.wYear) &&
			(tm1.wMonth == tm2.wMonth) &&
			(tm1.wDay > tm2.wDay)){
				return 1;
		}
		if((tm1.wYear == tm2.wYear) &&
			(tm1.wMonth == tm2.wMonth) &&
			(tm1.wDay == tm2.wDay) &&
			(tm1.wHour > tm2.wHour) ){
				return 1;
		}
		if((tm1.wYear == tm2.wYear) &&
			(tm1.wMonth == tm2.wMonth) &&
			(tm1.wDay == tm2.wDay) &&
			(tm1.wHour == tm2.wHour) &&
			(tm1.wMinute > tm2.wMinute)){
				return 1;
		}
		if((tm1.wYear == tm2.wYear) &&
			(tm1.wMonth == tm2.wMonth) &&
			(tm1.wDay == tm2.wDay) &&
			(tm1.wHour == tm2.wHour) &&
			(tm1.wMinute == tm2.wMinute) &&
			(tm1.wSecond > tm2.wSecond) ){
				return 1;
		}
		if((tm1.wYear == tm2.wYear) &&
			(tm1.wMonth == tm2.wMonth) &&
			(tm1.wDay == tm2.wDay) &&
			(tm1.wHour == tm2.wHour) &&
			(tm1.wMinute == tm2.wMinute) &&
			(tm1.wSecond == tm2.wSecond) ){
				return 0;
		}
		return -1;
	}
};

//文件操作常用函数
class MZ_CommonFile{
public:
	//检查文件夹是否存在，不存在则新建
	static BOOL DirectoryExists_New(TCHAR* foldername){
		if(FileExists(foldername)){
			return true;
		}
		return CreateDirectory(foldername,NULL);
	}
	//以当前时间为名称创建文件夹
	static CMzString CreateDirectoryByDate(TCHAR* parentdir,BOOL &result){
		CMzString str = parentdir;
		str = str + L"\\";
		str = str + MZ_CommonDateTime::NowtoStr();
		result = DirectoryExists_New(str.C_Str());
		return str;
	}
	//列表所有文件夹
	//无文件夹则返回0
	static int ListDirectory(TCHAR* parentdir, list<CMzString> &s){
	  WIN32_FIND_DATA FindFileData;
	  HANDLE hFind;
	  BOOL bFinished = true;
	  int ret = 0;
	  CMzString str = parentdir;
	  str = str + L"\\*.*";

	  s.clear();	//清空搜索结果
	  hFind = FindFirstFile(str.C_Str(), &FindFileData);
	  if(hFind != INVALID_HANDLE_VALUE){
		  while(bFinished){
			  if((FindFileData.dwFileAttributes & (~FILE_ATTRIBUTE_DIRECTORY)) == 0){	//is Directory
				  s.push_back(FindFileData.cFileName);
				  ret++;
			  }
			  bFinished = FindNextFile(hFind,&FindFileData);
		  }
		  FindClose(hFind);
	  }
	  return ret;
	}

	//备份所有文件
	//当目标文件夹不存在是，自动新建
	static bool BackupFiles(TCHAR* srcdir, TCHAR* destdir,list<CMzString> &files){
		if(lstrcmp(srcdir,destdir) == 0){	//源文件夹与目标文件夹相同
			return false;
		}
		if(!FileExists(srcdir)){	//源文件夹不存在
			return false;
		}
		if(!DirectoryExists_New(destdir)){ //目标文件夹创建错误
			return false;
		}
		list<CMzString>::iterator i = files.begin();
		for(; i != files.end(); i++){
			CMzString srcfile = srcdir;
			srcfile = srcfile + L"\\" + *i;
			CMzString destfile = destdir;
			destfile = destfile + L"\\" + *i;
			CopyFile(srcfile,destfile,false);
		}
		return true;
	}

	static BOOL DelFile(TCHAR* file){
		BOOL ret = false;
		if(FileExists(file)){
			ret = DeleteFile(file);
		}
		return ret;
	}
	//遍历删除文件夹
	static BOOL DelDirectory(TCHAR* foldername){
	  WIN32_FIND_DATA FindFileData;
	  HANDLE hFind;
	  BOOL ret;
	  BOOL bFinished = true;
	  CMzString str = foldername;
	  str = str + L"\\*.*";

	  CMzString parentfolder = foldername;
	  parentfolder = parentfolder + L"\\";

	  hFind = FindFirstFile(str.C_Str(), &FindFileData);
	  if(hFind != INVALID_HANDLE_VALUE){
		  while(bFinished){
			  if((FindFileData.dwFileAttributes & (~FILE_ATTRIBUTE_DIRECTORY)) == 0){	//is Directory
				  CMzString folder = parentfolder + FindFileData.cFileName;
				  ret = DelDirectory(folder.C_Str());
			  }else{
				  CMzString file = parentfolder + FindFileData.cFileName;
				  ret = DeleteFile(file.C_Str());
			  }
			  bFinished = FindNextFile(hFind,&FindFileData);
		  }
		  FindClose(hFind);
	  }
	  return RemoveDirectory(parentfolder.C_Str());
	}
	//检查文件否存在
	static bool FileExists(TCHAR* filename)
	{
	  WIN32_FIND_DATA FindFileData;
	  HANDLE hFind;

	  hFind = FindFirstFile(filename, &FindFileData);
	  if (hFind == INVALID_HANDLE_VALUE)
	  {
		return false;
	  }
	  else
	  {
		FindClose(hFind);
		return true;
	  }
	}
	//获取应用程序目录
	static bool GetCurrentPath(LPTSTR szPath) 
	{ 
			HMODULE handle = GetModuleHandle(NULL);
			DWORD dwRet = GetModuleFileName(handle, szPath, MAX_PATH);
			if (dwRet == 0)
			{
					return false;
			}
			else
			{
					TCHAR* p = szPath;
					while(*p)++p; //let p point to '\0' 
					while('\\' != *p)--p; //let p point to '\\' 
					++p;
					*p = '\0'; //get the path
					return true; 
			}
	}
	static TEXTENCODE_t getTextCode(TCHAR* filename);
	//按行载入文档
	static list<CMzString> loadText(TCHAR* file, TEXTENCODE_t enc = ttcAnsi);
	static wchar_t* chr2wch(const char* buffer, wchar_t** wBuf);
};

namespace MZ_CommonFunc {
	class File : public MZ_CommonFile {};
	class DateTime : public MZ_CommonDateTime {};
	class C : public MZ_CommonC { };
};