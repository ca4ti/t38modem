/*
 * main.h
 *
 * T38FAX Pseudo Modem
 *
 * Open H323 Project
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Open H323 Library.
 *
 * The Initial Developer of the Original Code is Vyacheslav Frolov
 *
 * Contributor(s): Equivalence Pty ltd
 *
 * $Log: main.h,v $
 * Revision 1.5  2002-02-12 11:25:29  vfrolov
 * Removed obsoleted code
 *
 * Revision 1.5  2002/02/12 11:25:29  vfrolov
 * Removed obsoleted code
 *
 * Revision 1.4  2002/01/10 06:10:02  craigs
 * Added MPL header
 *
 */

// mostly "stolen" from OpenAM

#ifndef _PM_MAIN_H
#define _PM_MAIN_H

#include <h323.h>
#include <lid.h>
#include "pmutils.h"

class T38Modem : public PProcess
{
  PCLASSINFO(T38Modem, PProcess)

  public:
    T38Modem();
    ~T38Modem();

    void Main();
    void RecordFile(PArgList & args);

  protected:
    long GetCodec(const PString & codecname);
    OpalLineInterfaceDevice * GetDevice(const PString & device);
};

class PseudoModem;
class PseudoModemQ;

class MyH323EndPoint : public H323EndPoint
{
  PCLASSINFO(MyH323EndPoint, H323EndPoint);

  public:
    MyH323EndPoint();

    // overrides from H323EndPoint
    virtual H323Connection * CreateConnection(unsigned callReference);
    BOOL OnIncomingCall(H323Connection &, const H323SignalPDU &, H323SignalPDU &);

    // new functions
    BOOL Initialise(PConfigArgs & args);
    void OnConnectionEstablished(H323Connection &, const PString &);

    BOOL ForceT38Mode() const { return forceT38Mode; }
    PseudoModem * PMAlloc() const;
    void PMFree(PseudoModem *pmodem) const;

  protected:
    BOOL forceT38Mode;
    PseudoModemQ *pmodemQ;
    PStringArray routes;
    WORD connectPort;

    PDECLARE_NOTIFIER(PObject, MyH323EndPoint, OnMyCallback);
};

class AudioRead;
class AudioWrite;
class OpalT38Protocol;

class MyH323Connection : public H323Connection
{
  PCLASSINFO(MyH323Connection, H323Connection);

  public:
    MyH323Connection(MyH323EndPoint &, unsigned);
    ~MyH323Connection();

    BOOL Attach(PseudoModem *_pmodem);

    // overrides from H323Connection
    AnswerCallResponse OnAnswerCall(const PString &, const H323SignalPDU &, H323SignalPDU &);
    BOOL OnStartLogicalChannel(H323Channel & channel);
    void OnClosedLogicalChannel(const H323Channel & channel);

    OpalT38Protocol * CreateT38ProtocolHandler() const;
    BOOL OpenAudioChannel(BOOL, unsigned, H323AudioCodec & codec);
    BOOL ForceT38Mode() const { return ep.ForceT38Mode(); }

  protected:
    const MyH323EndPoint & ep;
    
    PseudoModem *pmodem;
    OpalT38Protocol *t38handler;
    PMutex T38Mutex;
    
    PMutex  connMutex;

    AudioWrite * audioWrite;
    AudioRead * audioRead;
};

///////////////////////////////////////////////////////////////

class AudioDelay : public PObject
{ 
  PCLASSINFO(AudioDelay, PObject);
  
  public:
    AudioDelay();
    BOOL Delay(int time);
    void Restart();
    int  GetError();
 
  protected:
    PTime  previousTime;
    BOOL   firstTime;
    int    error;
};

class AudioRead : public PChannel
{
  PCLASSINFO(AudioRead, PChannel);

  public:
    AudioRead(MyH323Connection & conn);
    BOOL Read(void * buffer, PINDEX amount);
    BOOL Close();

  protected:
    MyH323Connection & conn;
    BOOL closed;
    AudioDelay delay;
    PMutex Mutex;

    BOOL triggered;
};

class AudioWrite : public PChannel
{
  PCLASSINFO(AudioWrite, PChannel)

  public:
    AudioWrite(MyH323Connection & conn);

    BOOL Write(const void * buf, PINDEX len);
    BOOL Close();

  protected:
    MyH323Connection & conn;
    BOOL closed;
    AudioDelay delay;
    PMutex Mutex;
};

#endif  // _PM_MAIN_H


// End of File ///////////////////////////////////////////////////////////////

