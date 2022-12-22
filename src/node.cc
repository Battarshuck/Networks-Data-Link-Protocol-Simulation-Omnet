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

#include "node.h"

using namespace std;

Define_Module(Node);

//*******************READING DATA************************
void Node::readData()
{
    //creating file name
    string fileName = "input" + to_string(getIndex()) + ".txt";

    ifstream inputFile;
    inputFile.open(fileName);

    string line, errorCode, text;

    //getting the the whole line
    while(getline(inputFile, line))
    {
        errorCode = line.substr(0, 4);
        text = line.substr(5, line.length() - 5);
        data.push_back(make_pair(errorCode, text));
    }

    inputFile.close();
}
//*******************************************************


//*******************ERROR DETECTION*********************
bool Node::checkParity(Message* msg)
{
        int seqNum = msg->getSeqNum();
        string payload = msg->getPayload();
        char trailer = msg->getTrailer();

        //initializing the parity with the sequence number
        bitset<8> parity(seqNum);

        //XORing each character in the payload with the parity
        for(int i = 0; i < payload.length(); ++i){
            parity = parity ^ bitset<8>(payload[i]);
        }

        //XORing parity with trailer
        parity = parity ^ bitset<8>(trailer);

        if(parity.to_ulong()==0){
            return true;
        }
        else
        {
            return false;
        }
}
//*******************************************************


//*******************FRAME CREATION***********************
char Node::createParity(string payload, seq_nr seqNum)
{

    bitset<8> parity(seqNum);
    for(int i = 0; i < payload.length(); ++i)
    {
        parity = parity ^ bitset<8>(payload[i]);
    }

//    cout << parity.to_string()<<endl;
    return (char)parity.to_ulong();

}
//********************************************************


//*******************BYTE STUFFING*************************
string Node::byteStuffing(string text)
{
    string payload = "";
    for(int i = 0; i < text.length(); ++i)
       {
           if(text[i] == '$')
               payload += '/';
           payload+=text[i];
       }
    return '$' + payload + '$';
}
//*********************************************************

//*******************CREATE FRAME**************************
Message* Node::createFrame(string text, seq_nr seqNum)
{
    string payload = byteStuffing(text);
    char parityByte = createParity(payload, seqNum);

    Message* msg = new Message();
    msg->setMessageType(1); //1 refers to normal message
    msg->setPayload(payload.c_str());
    msg->setSeqNum(seqNum);
    msg->setTrailer(parityByte);

    return msg;
}
//*********************************************************


//*******************PAYLOAD DESTUFFING*********************
string Node::byteDeStuffing(string payload)
{
    string message = "";
    for(int i = 1; i < payload.length()-1; ++i)
    {
        if(payload[i] == '/')
            continue;
        message += payload[i];
    }
    return message;
}
//******************Create Error****************************************

void Node::modification(Message*msg)
{
    string payload(msg->getPayload());
    int size=payload.length();
    bitset<8> forerror (1);
    vector<bitset<8>> vec;
    for(int i=0;i<size;i++)
    {
        bitset<8> xbits (payload[i]);
        vec.push_back(xbits);
    }

    int error=int(uniform(0,size));
    vec[error]=vec[error]^forerror;

    string collect="";

    for(int it=0;it<vec.size();it++)
    {
        collect=collect+(char)(vec[it]).to_ulong();
    }

    msg->setPayload(collect.c_str());

 }
//******************Receiver Function*************************************
void Node::rec(Message*msg)
{
    string type;
    if(msg->getSeqNum()==frameExpected)
    {
        if(checkParity(msg))
        {
            msg->setAckNum(frameExpected);
            inc(frameExpected);
            msg->setMessageType(ACK);
            recData.push_back(msg->getPayload());
            type="ACK";
        }
        else
        {
            msg->setAckNum(frameExpected);
            msg->setMessageType(NACK);
            type="NACK";
        }
        int Loss=int(uniform(0,100));
        double x = (double)simTime().dbl();
        if(Loss>=LossProb)
        {
            string s =to_string(msg->getAckNum());
            msg->setName(s.c_str());
            sendDelayed(msg, propagationDelay, "out");
            logs.log_ControlFrame(to_string(x), to_string(getIndex()), type, to_string(msg->getAckNum()), 0);
        }
        else
            logs.log_ControlFrame(to_string(x), to_string(getIndex()), type, to_string(msg->getAckNum()), 1);
    }
    else
    {
        //send ack of the last received frame
        msg->setAckNum(dec(frameExpected));
        double x = (double)simTime().dbl();
        msg->setMessageType(ACK);
        int Loss=int(uniform(0,100));
        if(Loss>=LossProb)
                {
        string s =to_string(msg->getAckNum());
        msg->setName(s.c_str());
        sendDelayed(msg, propagationDelay, "out");
        logs.log_ControlFrame(to_string(x), to_string(getIndex()),"ACK",to_string(msg->getAckNum()), 0);
                }
        else
            logs.log_ControlFrame(to_string(x), to_string(getIndex()),"ACK",to_string(msg->getAckNum()), 1);

    }


}


//*******************GO BACKN FUNCTIONS*********************
void Node::inc(seq_nr& currentSeqNum){
    if(currentSeqNum < maxSeqNum)
        currentSeqNum++;
    else
        currentSeqNum=0;
}
//**********************************************************
seq_nr Node::dec(seq_nr lastAck)
{
    if(lastAck>0)
        return lastAck-1;
    else
        return maxSeqNum;

 }
//**********************************************************
void Node::startTimer(seq_nr seqNum){
    Message* timerMessage = new Message("TIMEOUT");

    timerMessage->setPayload(to_string(seqNum).c_str());
    timerMessage->setMessageType(TIMEOUT_MSG);

    timerMessages[seqNum] = timerMessage;

    scheduleAt(simTime()+timeout, timerMessage);
}

//**********************************************************
void Node::stopTimer(seq_nr seqNum){
    cancelAndDelete(timerMessages[seqNum]);
}

//**********************************************************

ErrorType Node::checkErrorType(string errorString, Message* msg){
    if(errorString[1]=='1')
        return LOSS;
    if(errorString[0] == '1')
        modification(msg);
    if(errorString[2] == '1' && errorString[3] == '1')
        return DELAYED_AND_DUPLICATED;
    if(errorString[2] == '1')
        return DUPLICATED;
    if(errorString[3] == '1')
        return DELAYED;
    return CORRECT;
}

//**********************************************************

void Node::sender(Message*msg, bool isSelfMessage){
//    while(true){
        if(currentDataIndex==data.size())
            return;

        if(isSelfMessage)
        {
            if(msg->getMessageType() == TIMEOUT_MSG)
            {

            }
            else
            {
                string s = msg->getPayload();
                cout << s << endl << msg->getErrorType() << endl;
                cout << "-------------------------------------------------------------" << endl;
                if(msg->getErrorType() == DELAYED)
                {
                    msg->setErrorType(CORRECT);
                    sendDelayed(msg, errorDelay+transmissionDelay, "out");
                }
                else if(msg->getErrorType() == CORRECT)
                {
                    msg->setErrorType(CORRECT);
                    sendDelayed(msg, transmissionDelay, "out");
                }

            }

        }
        else
        {
            if(msg->getFrameType() == ACK)
            {
//                stopTimer(msg->getSeqNum());
                senderMsgBuffer.erase(senderMsgBuffer.begin());

            }
            else if(msg->getFrameType() == NACK)
            {

            }
            else
            {
                cout << "HENA" << endl;

            }

        }

        while(senderMsgBuffer.size() < senderWindowSize)
        {
            msg = createFrame(data[currentDataIndex].second, nextFrameToSend);
            // CHECK TYPES OF ERRORS
            senderMsgBuffer.push_back(msg); // Put it into buffer before modification
            ErrorType typesOfError= checkErrorType(data[currentDataIndex].first, msg);
            startTimer(nextFrameToSend);
            currentDataIndex++;
            inc(nextFrameToSend);
            if(typesOfError == LOSS)
            {
                cout << "LOSS" << endl;
                msg->setErrorType(LOSS);
                scheduleAt(simTime()+processingTime, msg);
            }
            else if(typesOfError == DELAYED)
            {
                cout << "DELAYED" << endl;
                msg->setErrorType(DELAYED);
                scheduleAt(simTime()+processingTime, msg);
            }
            else if(typesOfError == DELAYED_AND_DUPLICATED)
            {
                cout << "DELAYED_AND_DUPLICATED" << endl;
                Message* msgDup = new Message(*msg);
                cout << "ANA MSG DUB" << endl << "----------------------" << endl << msgDup->getPayload() << endl << "---------------" << endl;
                msg->setErrorType(DELAYED);
                msgDup->setErrorType(DELAYED);
                scheduleAt(simTime()+processingTime, msg);
                scheduleAt(simTime()+processingTime+duplicationDelay, msgDup);
            }
            else if(typesOfError == DUPLICATED)
            {
                cout << "DUPLICATED" << endl;
                Message* msgDup = new Message(*msg);
                cout << "ANA MSG DUB" << endl << "----------------------" << endl << msgDup->getPayload() << endl << "---------------" << endl;
                msgDup->setErrorType(CORRECT);
                msg->setErrorType(CORRECT);
                scheduleAt(simTime()+processingTime, msg);
                scheduleAt(simTime()+processingTime+duplicationDelay, msgDup);
            }
            else if(typesOfError == CORRECT)
            {
                cout << "CORRECT" << endl;
                msg->setErrorType(CORRECT);
                scheduleAt(simTime()+processingTime, msg);
            }

        }

//    }

}

//**********************************************************

void Node::initialize()
{
    // TODO - Generated method body
    myRole = RECEIVER;
    processingTime = par("PT").doubleValue();

    propagationDelay = par("PD").doubleValue();
    LossProb= par("LP").doubleValue();

    timeout = par("TO").doubleValue();
    errorDelay = par("ED").doubleValue();
    duplicationDelay = par("DD").doubleValue();
    transmissionDelay = par("TD").doubleValue();
    senderWindowSize = par("windowSize").intValue();
    maxSeqNum = senderWindowSize;
    currentDataIndex = 0;
    ackExpected = 0;
    nextFrameToSend = 0;
    isNetworkLayerReady = true;
    frameExpected=0;

}




void Node::handleMessage(cMessage *msg)
{
    Message* msg2 = check_and_cast<Message*>(msg);

    if(!msg->isSelfMessage()){
        if(msg2->getMessageType() == COORD_MSG && stoi(msg2->getPayload()) == getIndex()){
            myRole = SENDER;
            readData();

        }

        scheduleAt(simTime()+2.0, msg);
        if(myRole==SENDER)
        {

        }

    }
    if(myRole == SENDER){
        cout << msg->isSelfMessage() << endl;
        sender(msg2,msg->isSelfMessage());
    }else{
        cout << "ANA RECEIVER" << endl;
        cout << msg2->getSeqNum() << endl;

//


        // for (auto const& x : timerMessages)
        // {
            // std::cout << x.first  // string (key)

                      // << std::endl;
        // }
        // cout << "==============================" << endl;
        // msg2->setFrameType(ACK);
        // sendDelayed(msg2, 1.0, "out");

            // //**************************TEST***************************
            // string text = data[0].second;
            // msg2 = createFrame(text, 0);
            // modification(msg2);
            // cout<< msg2->getSeqNum()<<" "<<msg2->getPayload() << " "<<msg2->getTrailer()<<endl;
            // //******************************************************

            // msg2->setName("My role is a sender");
            // sendDelayed(msg2, 2.0, "out");

        // }else{

            // //**********************TEST*******************
            // if(checkParity(msg2))
                // cout << "correct" << endl;
            // else
                // cout << "error detected"<<endl;
            // //****************************************
            // msg2->setName("alooo");

            // rec(msg2);
            // //msg2->setName("My role is a receiver");
        // }
    }



//    else{
//
//        cout << "----------------------------" << endl;
//        cout<< "Sending at " << (double)(simTime().dbl()) << endl;
//        cout << "----------------------------" << endl;
//        msg2->setMessageType(NORMAL_MSG);
//        if(myRole == SENDER){
//
//            //**************************TEST***************************
//            string text = data[0].second;
//            msg2 = createFrame(text, 1);
//            modification(msg2);
//            cout<< msg2->getSeqNum()<<" "<<msg2->getPayload() << " "<<msg2->getTrailer()<<endl;
//            //******************************************************
//
//            msg2->setName("My role is a sender");
//        }else{
//
//            //**********************TEST*******************
//            if(checkParity(msg2))
//                cout << "correct" << endl;
//            else
//                cout << "error detected"<<endl;
//            //****************************************
//
//            msg2->setName("My role is a receiver");
//        }
//        sendDelayed(msg2, 2.0, "out");
//    }
}
