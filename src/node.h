//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __GOBACKN_NODE_H_
#define __GOBACKN_NODE_H_

#include <omnetpp.h>
#include "Variables.h"
#include <vector>
#include <fstream>
#include <utility>
#include "Message_m.h"
#include <bitset>
#include <map>
#include "LogsGenerator.h"
#include <string>

typedef int seq_nr;

using namespace omnetpp;
using namespace std;

/**
 * TODO - Generated class
 */

class Node : public cSimpleModule
{
    LogsGenerator logs;
    role myRole;

    cMessage* coordMsg;

    seq_nr maxSeqNum;
    seq_nr currentDataIndex;
    seq_nr nextFrameToSend;
    seq_nr ackExpected;
    seq_nr frameExpected=0;

    int receiverWindowSize = 1;
    int senderWindowSize;

    double processingTime;
    double LossProb;
    double timeout;
    double errorDelay;
    double duplicationDelay;
    double transmissionDelay;

    bool isNetworkLayerReady;

    map<seq_nr, Message*> timerMessages;

    vector<string> recData;
    vector<Message*> senderMsgBuffer;
    vector<pair<string, string>> data; //data.first -> error code e.g. 1011
                                       //data.second -> actual text
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    ~Node();

    void readData();
    bool checkParity(Message *msg);                   //check message parity (error detection)
    char createParity(string payload, seq_nr SeqNum);    //creates the parity byte in the trailer
    Message* createFrame(string text, seq_nr seqNum);    //creates the frame
    string byteStuffing(string text);                 //DONT USE (utility function)
    string byteDeStuffing(string payload);
    void modification(Message*msg);


    void sender(Message*msg, bool isSelfMessage);
    void startTimer(seq_nr seqNum);
    void stopTimer(seq_nr seqNum);
    void handlingMsgErrors(Message*msg, ErrorType typesOfError, double currentMsg);
    void resendBuffer();
    ErrorType checkErrorType(string errorString, Message* msg);
    void updateMessageStateInBuffer(seq_nr seqNum);
    void resetTimer();
    bool isAckWithInRange(seq_nr receivedAckNum);
    void fromNetworkLayer(Message *msg, double& currentMsg);
    void handleAcks(Message *msg);
    void toPhysicalLayer(Message *msg);

    void rec(Message*msg,bool isSelfMessage);

    seq_nr dec(seq_nr lastAck);
    void inc(seq_nr& currentSeqNum);
};

#endif
