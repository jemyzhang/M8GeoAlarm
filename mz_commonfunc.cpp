#include "mz_commonfunc.h"
#include <fstream>
using namespace std;

int MZ_CommonDateTime::getWeekDay(int year,int month, int day){
	int D,M,Y,A; 
	D = day; M = month; Y = year;
	if((M==1)||(M==2)) //1,2月当作前一年的13,14月 
	{ 
		M+=12; 
		Y--; 
	} 
	if((Y<1752)||((Y==1752)&&(M<9))||((Y==1752)&&(M==9)&&(D<3))){ //判断是否在1752年9月3日之前 
		A=(D+2*M+3*(M+1)/5+Y+Y/4+5)%7; //1752年9月3日之前的公式 
	}else{ 
		A=(D+2*M+3*(M+1)/5+Y+Y/4-Y/100+Y/400)%7; //1752年9月3日之后的公式 
	}
	return A;
}

int Days[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int MZ_CommonDateTime::getDays(int year, int month){
	int day = 0;
	if(year < 1970 || (year > 2199)) return day;

	if((month < 1) || (month > 12))   return day;   
    
	isLeapyear(year);

	return Days[month-1];
}

bool MZ_CommonDateTime::isLeapyear(int year){
	bool b_IsLeapYear = false;
	if((year%4 == 0 && year%100 != 0) || (year%400 == 0))     
	{   
		b_IsLeapYear=true;    
	}   
    
	if(b_IsLeapYear){
		Days[1] = 29;
	}else{
		Days[1] = 28;
	}
	return b_IsLeapYear;
}

void MZ_CommonDateTime::OneDayDate(SYSTEMTIME &date,bool isYesterday){
	isLeapyear(date.wYear);
	if(isYesterday){
		date.wDay --;
		if(date.wDay <= 0){
			date.wMonth --;
			if(date.wMonth <= 0){
				date.wYear --;
				isLeapyear(date.wYear);
				date.wMonth = 12;
			}
			date.wDay = Days[date.wMonth - 1];
		}
	}else{
		date.wDay++;
		if(date.wDay > Days[date.wMonth - 1]){
			date.wDay = 1;
			date.wMonth ++;
			if(date.wMonth > 12){
				date.wMonth = 1;
				date.wYear++;
			}
		}
	}
}

//error: 1: year, 2: month, 4: day, 0: no error
int MZ_CommonDateTime::checkDate(int year,int month, int day)   
{
	int rc = 0;
	if(year < 1970 || (year > 2199)) rc |= 1;

	if((month < 1) || (month > 12))   rc |= 2;   
    
	isLeapyear(year);

	if((day < 0) || (day > Days[month-1])){
		rc |= 4;
	}
    
	return rc;   
} 

void MZ_CommonC::newstrcpy(wchar_t** pdst,const wchar_t* src){
	if(*pdst) delete *pdst;
	wchar_t* newdst = new wchar_t[lstrlen(src) + 1];
	lstrcpy(newdst,src);
	*pdst = newdst;
}

wchar_t* MZ_CommonC::removeWrap(wchar_t* dst, wchar_t* src){
	*dst = '\0';
	if(src == NULL) return dst;
	if(lstrlen(src) == 0) return dst;
	wchar_t c;
	wchar_t *p = src;
	wchar_t *d = dst;
	do{
		c = *p++;
		if(c == '\n'){
			*d++ = '\\';
			*d++ = 'n';
		}else if(c == '\r'){
			*d++ = '\\';
			*d++ = 'r';
		}else{
			*d++ = c;
		}
	}while(c);
	*d = '\0';
	return dst;
}

wchar_t* MZ_CommonC::restoreWrap(wchar_t* dst, wchar_t* src){
	*dst = '\0';
	if(src == NULL) return dst;
	if(lstrlen(src) == 0) return dst;
	wchar_t c;
	wchar_t *p = src;
	wchar_t *d = dst;
	bool isplash = false;
	do{
		c = *p++;
		if(isplash){
			isplash = false;
			if(c == 'r'){
				*d++ = '\r';
			}else if(c == 'n'){
				*d++ = '\n';
			}else{
				*d++ = '\\';	//补充置位的符号
				*d++ = c;
			}
		}else{
			if(c == '\\'){
				isplash = true;
			}else{
				*d++ = c;
			}
		}
	}while(c);
	*d = '\0';
	return dst;
}

TEXTENCODE_t MZ_CommonFile::getTextCode(TCHAR* filename)
{
	TEXTENCODE_t ret;
    if (FileExists(filename))
    {
		fstream openfile;
        openfile.open(filename, ios::in | ios::binary);
        openfile.seekg(0, ios::beg);
        char *code = new char[2];
        openfile.read(code, 2);
        openfile.close();
        
        unsigned char cc = (unsigned char)code[0];
        unsigned char cc1 = (unsigned char)code[1];
        if ((cc == 0xFF) && (cc1 == 0xFE))
        {
            ret = ttcUnicode;
        }
        else if ((cc == 0xFE) && (cc1 == 0xFF))
        {
            ret = ttcUnicodeBigEndian;
        }
        else if ((cc == 0xEF) && (cc1 == 0xBB))
        {
            ret = ttcUtf8;
        }
        else
        {
            ret = ttcAnsi;
        }
    }
	return ret;
}

wchar_t* MZ_CommonFile::chr2wch(const char* buffer, wchar_t** wbuf)
{
      size_t len = strlen(buffer); 
      size_t wlen = MultiByteToWideChar(CP_ACP, 0, (const char*)buffer, int(len), NULL, 0); 
      wchar_t *wBuf = new wchar_t[wlen + 1]; 
      MultiByteToWideChar(CP_ACP, 0, (const char*)buffer, int(len), wBuf, int(wlen));
	  *wbuf = wBuf;
	  return wBuf;
} 

list<CMzString> MZ_CommonFile::loadText(TCHAR* filename, TEXTENCODE_t enc){
	list<CMzString> lines;
	if(!FileExists(filename)){
		return lines;
	}

	CMzString m_Text;
	if(enc == ttcAnsi){
        ifstream file;
        file.open(filename,  ios::in | ios::binary);
        if (file.is_open())
        {
                file.seekg(0, ios::end);
                int nLen = file.tellg();
                char *ss = new char[nLen+1];
                file.seekg(0, ios::beg);
                file.read(ss, nLen);
                ss[nLen] = '\0';
				wchar_t *wss;
				chr2wch(ss,&wss);
                m_Text = wss;
				delete[] ss;
				delete[] wss;
        }
        file.close();
	}else if(enc == ttcUnicode ||
		enc == ttcUnicodeBigEndian){
		wifstream ofile;
        ofile.open(filename, ios::in | ios ::binary);
        if (ofile.is_open())
        {
			ofile.seekg(0, ios::end);
            int nLen = ofile.tellg();
            ofile.seekg(2, ios::beg);
            wchar_t *tmpstr = new wchar_t[nLen + 1];
            ofile.read(tmpstr, nLen);                        
            tmpstr[nLen] = '\0';
            m_Text = tmpstr;
			delete[] tmpstr;
		}
        ofile.close();
	}

	//处理成多行
	wchar_t *pstr = m_Text.C_Str();
	wchar_t wch = 0;
	int scnt = 0;
	int npos = 0;
	int ncnt = 0;
	do{
		wch = pstr[scnt++];
		if(wch == '\n' || wch == '\r'){
			if(ncnt != 0){
				lines.push_back(m_Text.SubStr(npos,ncnt));
				npos += (ncnt+1);
				ncnt = 0;
			}else{
				npos++;	//忽略换行符
			}
			continue;
		}
		ncnt++;
	}while(wch != '\0');
	return lines;
}