#include "FaceApp.h"
#include "windows.h"
#include "FTTypes.h"
#include <QDebug>

//
// Override the Application MessageFilter, to receive messages from the game(s)
//
bool FaceApp::winEventFilter( MSG * msg, long * result )
{
	int msgType = msg->message;  // test line
	
	if (msgType == msgID_FTClient) {
		qDebug() << "FaceApp::winEventFilter says: game tickles me =" << msgType << "hwnd =" << msg->hwnd;
		if (mainWindow != NULL) {
			mainWindow->getGameProgramName();
		}
	}
		
	return( false );
}

//
// Setup the EventFilter
//
void FaceApp::SetupEventFilter( FaceTrackNoIR *window ) {

	mainWindow = window;
	msgID_FTClient = RegisterWindowMessageA ( FT_PROGRAMID );

	qDebug() << "FaceApp::SetupEventFilter says: Message ID =" << msgID_FTClient;

}