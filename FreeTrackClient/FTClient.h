/**   @file  
     @brief  
*/
#pragma once
#ifndef INCLUDED_FTCLIENT_H
#define INCLUDED_FTCLIENT_H
 




  
#include "Windows.h" 
#include "SysUtils.h" 
#include "FTTypes.h"

/*__stdcall*/ bool FTGetData(PFreetrackData data); 
/*__stdcall*/ void FTReportName(PAnsiChar name); 
/*__stdcall*/ char* FTGetDllVersion(); 
/*__stdcall*/ char* FTProvider(); 


bool OpenMapping();
void DestroyMapping();




#endif//INCLUDED_FTCLIENT_H
//END
