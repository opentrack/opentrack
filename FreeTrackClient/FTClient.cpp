/**   @file  
     @brief  
*/
#include "FTClient.h"




  static double/*?*/ const FT_PROGRAMID = "FT_ProgramID";


  HANDLE hFTMemMap;
  PFreetrackData FTData;
  unsigned long lastDataID;
  PHandle FTHandle;
  PAnsiChar FTProgramName;
  HANDLE FTMutex;


bool FTGetData(PFreetrackData data)
{   bool result;
  result = false;
  if(  !! FTData ) {
    if(  FTData->DataID != lastDataID ) {
      Move( FTData/*?*/^ , data/*?*/^ , SizeOf( TFreetrackData ) );
      lastDataID = FTData->DataID;
      result = true;
    }
  } else
    OpenMapping;
return result;
}


void FTReportName(PAnsiChar name)
{
  unsigned long MsgResult;

  if(  OpenMapping && ( WaitForSingleObject( FTMutex , 100 ) == WAIT_OBJECT_0 ) ) {
    Move( name/*?*/^ , FTProgramName/*?*/^ , 100 );
    SendMessageTimeout( FTHandle/*?*/^ , RegisterWindowMessage( FT_PROGRAMID ) , 0 , 0 , 0 , 2000 , MsgResult );
    ReleaseMutex( FTMutex );
  }
}


char* FTGetDllVersion()
{   char* result;
  unsigned long VerInfoSize;
  Pointer VerInfo;
  unsigned long VerValueSize;
  PVSFixedFileInfo VerValue;
  unsigned long Dummy;
  std::string verString;
  char* dllName[100];

  result = "";
  GetModuleFilename( HInstance , &dllName , 100 );
  VerInfoSize = GetFileVersionInfoSize( &dllName , Dummy );
  if(  !( VerInfoSize == 0 ) ) {
    GetMem( VerInfo , VerInfoSize );
    GetFileVersionInfo( &dllName , 0 , VerInfoSize , VerInfo );
    VerQueryValue( VerInfo , "\\" , Pointer( VerValue ) , VerValueSize );
    /*?*//* WITH  VerValue/*?*/^ */
    {
      verString = IntToStr( dwFileVersionMS >> 16 );
      verString = verString + "." + IntToStr( dwFileVersionMS && 0xFFFF );
      verString = verString + "." + IntToStr( dwFileVersionLS >> 16 );
      verString = verString + "." + IntToStr( dwFileVersionLS && 0xFFFF );
      result = char*( verString );
    }
    FreeMem( VerInfo , VerInfoSize );
  }
return result;
}


char* FTProvider()
{   char* result;
  result = FREETRACK;return result;
}





bool OpenMapping()
{   bool result;
  if(  hFTMemMap != 0 )
    result = true;else {
    hFTMemMap = OpenFileMapping( FILE_MAP_ALL_ACCESS , false , FT_MM_DATA );
    if(  ( hFTMemMap != 0 ) ) {
      FTData = MapViewOfFile( hFTMemMap , FILE_MAP_ALL_ACCESS , 0 , 0 , SizeOf( TFreetrackData ) + SizeOf( HANDLE ) + 100 );
      FTHandle = Pointer( unsigned long( FTData ) + SizeOf( TFreetrackData ) );
      FTProgramName = Pointer( unsigned long( FTHandle ) + SizeOf( HANDLE ) );
      FTMutex = OpenMutex( MUTEX_ALL_ACCESS , false , FREETRACK_MUTEX );
    }
    result = !! FTData;
  }
return result;
}


void DestroyMapping()
{
  if(  FTData != 00 ) {
    UnMapViewofFile( FTData );
    FTData = 00;
  }

  CloseHandle( FTMutex );
  CloseHandle( hFTMemMap );
  hFTMemMap = 0;
}




//END
