#include "FaceApp.h"
#include "windows.h"
#include "FTTypes.h"
#include "..\FTNoIR_Protocol_FTIR\FTIRTypes.h"
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

	////if (msgType == WM_HOTKEY) {
	////	switch ( msg->wParam ) {
	////		case 777:
	////			qDebug() << "FaceApp::winEventFilter says: HOME pressed";
	////			break;
	////		case 778:
	////			qDebug() << "FaceApp::winEventFilter says: END pressed";
	////			break;
	////		default:
	////			qDebug() << "FaceApp::winEventFilter says: unknown HotKey pressed";
	////			break;
	////	}
	////}
	return( false );
}

//
// Setup the EventFilter
//
void FaceApp::SetupEventFilter( FaceTrackNoIR *window ) {

	mainWindow = window;
	msgID_FTClient = RegisterWindowMessageA ( FT_PROGRAMID );
	qDebug() << "FaceApp::SetupEventFilter says: Message ID =" << msgID_FTClient;
	msgID_FTIR_Register = RegisterWindowMessageA ( FTIR_REGISTER_PROGRAMHANDLE );
	msgID_FTIR_UnRegister = RegisterWindowMessageA ( FTIR_UNREGISTER_PROGRAMHANDLE );

	////if ( RegisterHotKey( window->winId(), 777, MOD_WIN, VK_HOME ) ) {
	////	qDebug() << "FaceApp::SetupEventFilter says: RegisterHotKey HOME =" << VK_HOME;
	////}
	////if ( RegisterHotKey( window->winId(), 778, MOD_WIN, VK_END ) ) {
	////	qDebug() << "FaceApp::SetupEventFilter says: RegisterHotKey END =" << VK_END;
	////}
	////
	////QAbstractEventDispatcher *evtdis = QAbstractEventDispatcher::instance();
	////if (evtdis != NULL) {
	////	qDebug() << "FaceApp::SetupEventFilter says: EventDispatcher found!";
	////}

}