#include "juke.h"
#include <System/SerialMgr.h>
#include <System/SysAll.h>
#include <UI/UIAll.h>

FieldPtr        fieldptr_text;   // FieldPtr and DmOpenRef are defined in the PalmOS header files.
FieldPtr        fieldptr_hex;    // See if you can find where they are defined.

UInt libRef;
#define BAUDRATE 57600

char		hex[60];


#define	Tex2HexAppID	'TxHx'
#define	Tex2HexDBType	'Data'

static Boolean OpenSerial(void)
{

  Err error;

  error = SysLibFind("Serial Library", &libRef);
  if (0 == error){
  	error = SerOpen(libRef, 0, BAUDRATE);
  }
  if (0 == error){
	return 0;
  }else if (error == serErrAlreadyOpen ) {
	return 0;
  }
  return error;
  
}

static int StartApplication(void)
{
  int error;

  error = OpenSerial();
  if (error) return error;

  FrmGotoForm(formID_juke);
}


static void StopApplication(void)
{
  FldSetTextHandle(fieldptr_text, NULL);
  SerClose(libRef);
}

static void Send_serial(void){
  char		textchar;
  int		textlength;
  CharPtr	textpointer;
  Err		error;
  
  textpointer = FldGetTextPtr(fieldptr_text);
  textlength = FldGetTextLength(fieldptr_text);
  if (textlength){
    SerSend(libRef, textpointer, textlength, &error);
    if ( error == serErrTimeOut ){
	FrmAlert(alertID_senderror);
    }
  }else{
     FrmAlert(alertID_errornullstring);
  }
}

static void Convert(void)
{
  char     textchar;
  int      textpos;
  int      lownybble, hinybble;
  char     hexchar;
  int      textlength;
  CharPtr  textpointer;

  textpointer = FldGetTextPtr(fieldptr_text);
  textlength = FldGetTextLength(fieldptr_text);
  if (textlength)
  {
    for (textpos = 0; textpos <textlength; textpos++)
    {
      textchar = textpointer[textpos];
      lownybble = textchar % 16;
      hinybble = textchar / 16;
      if (hinybble < 10) hexchar = 48 + hinybble;
        else hexchar = 55 + hinybble;
      hex[textpos * 3] = hexchar;
      if (lownybble < 10) hexchar = 48 + lownybble;
        else hexchar = 55 + lownybble;
      hex[textpos*3 + 1] = hexchar;
      hex[textpos * 3 + 2] = 45;
    }
    hex[textpos * 3 - 1] = 0;
    FldSetTextPtr(fieldptr_hex, hex);
    FldRecalculateField(fieldptr_hex, true);
    FldDrawField(fieldptr_hex);
  }
  else
  {
    FrmAlert(alertID_errornullstring);
  }
}

static void ChangeBitmap(void)
{
  VoidHand   bitmaphandle;
  BitmapPtr  bitmap;
  int	     WhichBitmap = 0;

  WhichBitmap = (WhichBitmap + 1) % 2;
  if (WhichBitmap == 0) bitmaphandle = DmGet1Resource('Tbmp', bitmapID_text);
  else bitmaphandle = DmGetResource('Tbmp', bitmapID_hex);
  bitmap = MemHandleLock(bitmaphandle);
  WinDrawBitmap(bitmap, 20, 120);
  MemHandleUnlock(bitmaphandle);
}

static Boolean jukeHandler(EventPtr event)
{
  Boolean       handled = 0;
  FormPtr   form;

  switch (event->eType) 
  {
  case    frmOpenEvent:
    form = FrmGetActiveForm();
    fieldptr_text = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fieldID_text));
    fieldptr_hex = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fieldID_hex));
    FrmDrawForm(form);
    handled = 1;
    break;
    
  case ctlSelectEvent:  // A control button was pressed and released.
    switch (event->data.ctlEnter.controlID){
    case buttonID_convert:    
      Convert();
      handled = 1;
      break;
    case buttonID_send:
      Send_serial();
      handled = 1;
    }
    break;

  case menuEvent:
    switch (event->data.menu.itemID)
    {
    case menuitemID_convert:
      Convert();
      break;
    case menuitemID_about:
      FrmAlert(alertID_about);
      break;
    case menuitemID_copy:
      FldCopy(fieldptr_text);   // user has to select (highllight) the text to copy
      break;
    case menuitemID_paste:
      FldPaste(fieldptr_text);
      break;
    }
    handled = 1;
    break;

case nilEvent:
    ChangeBitmap();
    handled = 1;
    break;
  }
  return handled;
}

static void EventLoop(void)
{
  short      err;
  int        formID;
  FormPtr    form;
  EventType  event;

  do
  {

    EvtGetEvent(&event, 200);

    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent((void *)0, &event, &err)) continue;

    if (event.eType == frmLoadEvent)
    {
      formID = event.data.frmLoad.formID;
      form = FrmInitForm(formID);
      FrmSetActiveForm(form);
      switch (formID) 
      {
      case formID_juke:
        FrmSetEventHandler(form, jukeHandler);
        break;
      }
    }
    FrmDispatchEvent(&event);
  } while(event.eType != appStopEvent);
}

DWord  PilotMain (Word cmd, Ptr cmdPBP, Word launchFlags)
{
  int error;

  if (cmd == sysAppLaunchCmdNormalLaunch)
 {

    error = StartApplication();     // Application start code
    if (error) return error;

    EventLoop();                    // Event loop

    StopApplication ();             // Application stop code
  }
  return 0;
}

