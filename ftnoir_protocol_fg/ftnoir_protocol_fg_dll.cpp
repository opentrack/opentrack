/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*																				*
********************************************************************************/
#include "ftnoir_protocol_fg.h"
#include <QDebug>
#include "facetracknoir/global-settings.h"

FTNoIR_ProtocolDll::FTNoIR_ProtocolDll() {
}

FTNoIR_ProtocolDll::~FTNoIR_ProtocolDll()
{

}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol object.

// Export both decorated and undecorated names.
//   GetProtocolDll     - Undecorated name, which can be easily used with GetProcAddress
//						Win32 API function.
//   _GetProtocolDll@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetProtocolDll=_GetProtocolDll@0")

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
    return new FTNoIR_ProtocolDll;
}
