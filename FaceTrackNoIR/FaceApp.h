#ifndef FACEAPP_H
#define FACEAPP_H

#include <QApplication>
#include "FaceTrackNoIR.h"

class FaceApp : public QApplication
{
    Q_OBJECT
public:
    FaceApp( int &argc, char **argv ) : QApplication( argc, argv ) {}
    ~FaceApp() {}

	void SetupEventFilter( FaceTrackNoIR *window );

protected:
    bool winEventFilter( MSG * msg, long * result );

private:
	FaceTrackNoIR *mainWindow;	
	int msgID_FTClient;
	int msgID_FTIR_Register;
	int msgID_FTIR_UnRegister;
};

#endif					//  FACEAPP_H
