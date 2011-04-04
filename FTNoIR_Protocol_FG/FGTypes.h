/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
*																				*
* Type definitions for the FlightGear server.									*
********************************************************************************/
#pragma once
#ifndef INCLUDED_FGTYPES_H
#define INCLUDED_FGTYPES_H

#include "Windows.h" 

//
// x,y,z position in metres, heading, pitch and roll in degrees...
//
#pragma pack(2)
struct TFlightGearData {
	double x, y, z, h, p, r;
	int status;
};

#endif//INCLUDED_FGTYPES_H
