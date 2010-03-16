#include <mzfc_inc.h>


bool CreateCellInfoInstance();
bool FinalizeCellInfoInstance();
bool ReqCurrentCellInfo(HWND m_hwnd);
bool GetCurrentCellInfo(DWORD *mcc, DWORD *mnc, DWORD *lac, DWORD *cid);