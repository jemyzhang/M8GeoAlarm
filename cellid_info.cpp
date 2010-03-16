#include "cellid_info.h"
#include "ril.h"
#include "logout.h"
#define MZ_WM_UI_REMINDER_UPDATE					MZFC_WM_MESSAGE+0x0100
typedef struct tagCellInfo {
    DWORD dwMobileCountryCode;          // @field TBD
    DWORD dwMobileNetworkCode;          // @field TBD
    DWORD dwLocationAreaCode;           // @field TBD
    DWORD dwCellID;                     // @field TBD
    DWORD dwBaseStationID;              // @field TBD
} CELLINFO, *LPCELLINFO;


CELLINFO g_result = {0};
HRESULT g_hCellTowerInfo;   
HRIL    g_RilHandle = NULL;
HINSTANCE g_RilInstance = NULL;
bool g_ReqCellInfoUpdated = false;
HWND g_ReqHwnd = 0;

typedef HRESULT (*Dll_RIL_Initialize_T)(DWORD, RILRESULTCALLBACK, RILNOTIFYCALLBACK, DWORD, DWORD, HRIL*);
typedef HRESULT (*Dll_RIL_DeInitialize_T)(HRIL);
typedef HRESULT (*DLL_RIL_GetCellTowerInfo_T)(HRIL);


Dll_RIL_Initialize_T           DLL_RIL_Initialize = NULL;
Dll_RIL_DeInitialize_T        DLL_RIL_Deinitialize = NULL;
DLL_RIL_GetCellTowerInfo_T       DLL_RIL_GetCellTowerInfo = NULL;


void RILResultCallback(DWORD dwResultCode, HRESULT hrCommandID, const void* pData, DWORD dwDataSize, DWORD dwParam)   
{   
    HRESULT hr = RIL_RESULT_OK;   

    switch ( dwResultCode )    
    {    
    case RIL_RESULT_OK:    
        {   
            if ( hrCommandID == g_hCellTowerInfo )   
            {   
                RILCELLTOWERINFO  *pCallInfo = ((RILCELLTOWERINFO *)pData);    
                // must call RIL_GetCellTowerInfo
                // This structure stores cell tower information.   
                g_result.dwMobileCountryCode = pCallInfo->dwMobileCountryCode;
                g_result.dwMobileNetworkCode = pCallInfo->dwMobileNetworkCode;
                g_result.dwCellID = pCallInfo->dwCellID;
                g_result.dwLocationAreaCode = pCallInfo->dwLocationAreaCode;
                g_result.dwBaseStationID = pCallInfo->dwBaseStationID;
                logout("RILResultCallback,( 0x%x 0x%x 0x%x 0x%x 0x%x )\n", 
                    g_result.dwMobileCountryCode, 
                    g_result.dwMobileNetworkCode,
                    g_result.dwCellID,
                    g_result.dwLocationAreaCode,
                    g_result.dwBaseStationID); 
                g_ReqCellInfoUpdated = false;   //更新完毕
                PostMessage(g_ReqHwnd,MZ_WM_UI_REMINDER_UPDATE,NULL,NULL);
            }   
            break;    
        }   
    }   
    return;   
}   

void RILNotifyCallback(   
                       DWORD dwCode,   
                       const void* lpData,        
                       DWORD cbData,             
                       //                 pointed to lpData   
                       DWORD dwParam)   
{   
    HRESULT hr = RIL_RESULT_OK;   
    logout("RILNotifyCallback,( 0x%x )\n", dwCode );   
}   

//Initialize RIL   
HRESULT Initialize( void )   
{   
    HRESULT hr = S_FALSE;   

    logout("RIL_Initialize\n");
    hr = DLL_RIL_Initialize( 1, RILResultCallback,    
        RILNotifyCallback, RIL_NCLASS_ALL, 0, &g_RilHandle );   

    if ( FAILED( hr ) )   
    {   
        logout("RIL_Initialize failed, hr = %x\n", hr);   
    }   

    g_hCellTowerInfo = S_FALSE;   
    return hr;   
}   

//deinitialize RIL   
HRESULT Deinitialize( void )   
{   
    HRESULT hr = S_FALSE; 

    logout("RIL_Deinitialize\n");

    hr = DLL_RIL_Deinitialize( g_RilHandle );   
    if ( FAILED( hr ) )   
    {   
        logout("RIL_Deinitialize failed, hr = %x\n", hr);   
    }   
    g_RilHandle = NULL;   

    return hr;   
}   

bool CreateCellInfoInstance(){
    bool retval = true;
    if(g_RilInstance == NULL){
        g_RilInstance = LoadLibrary(_T("ril.dll"));

        DLL_RIL_Initialize = (Dll_RIL_Initialize_T)GetProcAddress(g_RilInstance, TEXT("RIL_Initialize"));
        DLL_RIL_Deinitialize = (Dll_RIL_DeInitialize_T)GetProcAddress(g_RilInstance, TEXT("RIL_Deinitialize"));
        DLL_RIL_GetCellTowerInfo = (DLL_RIL_GetCellTowerInfo_T)GetProcAddress(g_RilInstance, TEXT("RIL_GetCellTowerInfo"));

        if(DLL_RIL_Initialize && DLL_RIL_Deinitialize && DLL_RIL_GetCellTowerInfo){
            retval = Initialize();
        }else{
            FreeLibrary(g_RilInstance);
            g_RilInstance = NULL;
            retval = false;
        }
    }
    return retval;
}

bool FinalizeCellInfoInstance(){
    bool retval = true;
    if(g_RilInstance){
        retval = Deinitialize();
        FreeLibrary(g_RilInstance);
        g_RilInstance = NULL;
    }
    return retval;
}

bool ReqCurrentCellInfo(HWND m_hwnd){
    g_ReqHwnd = m_hwnd;
    logout("RIL_ReqCurrentCellInfo\n");
    if(g_ReqCellInfoUpdated){   //requesting
        return false;
    }else{
        g_ReqCellInfoUpdated = true;
        g_hCellTowerInfo = DLL_RIL_GetCellTowerInfo(g_RilHandle);
        return true;
    }
}

bool GetCurrentCellInfo(DWORD *mcc, DWORD *mnc, DWORD *lac, DWORD *cid){
    logout("RIL_GetCurrentCellInfo\n");
    bool retval = true;
    if(g_RilInstance == NULL) return false;

    if(g_ReqCellInfoUpdated){ //请求中，尚未返回结果
        retval = false;
    }
    if(mcc) *mcc = g_result.dwMobileCountryCode;
    if(mnc) *mnc = g_result.dwMobileNetworkCode;
    if(lac) *lac = g_result.dwLocationAreaCode;
    if(cid) *cid = g_result.dwCellID;
    return retval;
}