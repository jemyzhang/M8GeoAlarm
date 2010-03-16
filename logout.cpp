#include "logout.h"

#define DISABLE_LOG 1

FILE* flog = NULL;

bool wlogout(const wchar_t* _Format,...){
#if DISABLE_LOG
    return true;
#endif
    if(flog == NULL){
        flog = _wfopen(L"\\Disk\\geoalarm.log",L"a+");
    }
    if(flog == NULL) return false;
    //获取log时间
    SYSTEMTIME tm;
    GetLocalTime(&tm);

    //记录时间
    fwprintf(flog,
        L"[%04d-%02d-%02d %02d:%02d:%02d] ",
        tm.wYear,tm.wMonth,tm.wDay,
        tm.wHour,tm.wMinute,tm.wSecond);

    wchar_t buf[1024];
    va_list ap;
    va_start(ap, _Format);
    vswprintf(buf,_Format,ap);
    fwprintf(flog,buf);
    va_end(ap);

    fwprintf(flog,L"\n");

    fclose(flog);
    flog = NULL;
    return true;
}

bool logout(const char* _Format,...){
#if DISABLE_LOG
    return true;
#endif
    if(flog == NULL){
        flog = fopen("\\Disk\\geoalarm.log","a+");
    }
    if(flog == NULL) return false;
    //获取log时间
    SYSTEMTIME tm;
    GetLocalTime(&tm);

    //记录时间
    fprintf(flog,
        "[%04d-%02d-%02d %02d:%02d:%02d] ",
        tm.wYear,tm.wMonth,tm.wDay,
        tm.wHour,tm.wMinute,tm.wSecond);

    char buf[1024];
    va_list ap;
    va_start(ap, _Format);
    vsprintf(buf,_Format,ap);
    fprintf(flog,buf);
    va_end(ap);

    fprintf(flog,"\n");

    fclose(flog);
    flog = NULL;
    return true;
}
