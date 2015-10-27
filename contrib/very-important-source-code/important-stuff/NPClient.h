
extern int NP_RegisterWindowHandle (HWND hwnd);
extern int NP_UnregisterWindowHandle (void);
extern int NP_RegisterProgramProfileID (unsigned short id);
extern int NP_QueryVersion (unsigned short *version);
extern int NP_RequestData (unsigned short req);
extern int NP_GetSignature (tir_signature_t *sig);
extern int NP_GetData (tir_data_t *data);
extern int NP_GetParameter (void);
extern int NP_SetParameter (void);
extern int NP_StartCursor (void);
extern int NP_StopCursor (void);
extern int NP_ReCenter (void);
extern int NP_StartDataTransmission (void);
extern int NP_StopDataTransmission (void);


