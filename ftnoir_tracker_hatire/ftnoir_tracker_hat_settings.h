/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_HAT_SETTINGS_H
#define FTNOIR_TRACKER_HAT_SETTINGS_H

#include <QString>

//-----------------------------------------------------------------------------
struct TrackerSettings
{	

	QString SerialPortName;
	bool EnableRoll;
	bool EnablePitch;
	bool EnableYaw;
	bool EnableX;
	bool EnableY;
	bool EnableZ;

	bool InvertRoll;
	bool InvertPitch;
	bool InvertYaw;
	bool InvertX;
	bool InvertY;
	bool InvertZ;


    int RollAxis;
    int PitchAxis;
    int YawAxis;
    int XAxis;
    int YAxis;
    int ZAxis;

	void load_ini();
	void save_ini() const;
};

#endif //FTNOIR_TRACKER_HAT_SETTINGS_H
