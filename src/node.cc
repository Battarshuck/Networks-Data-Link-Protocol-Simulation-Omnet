#include "node.h"

using namespace std;

Define_Module(Node);

//*******************READING DATA************************
void Node::readData()
{
    // creating file name
    string fileName = "input" + to_string(getIndex()) + ".txt";

    ifstream inputFile;
    inputFile.open(fileName);

    string line, errorCode, text;

    // getting the the whole line
    while (getline(inputFile, line))
    {
        errorCode = line.substr(0, 4);
        text = line.substr(5, line.length() - 5);
        data.push_back(make_pair(errorCode, text));
    }
    inputFile.close();
}
//*******************************************************

//*******************ERROR DETECTION*********************
bool Node::checkParity(Message *msg)
{
    int seqNum = msg->getSeqNum();
    string payload = msg->getPayload();
    char trailer = msg->getTrailer();

    // initializing the parity with the sequence number
    bitset<8> parity(seqNum);

    // XORing each character in the payload with the parity
    for (int i = 0; i < payload.length(); ++i)
    {
        parity = parity ^ bitset<8>(payload[i]);
    }

    // XORing parity with trailer
    parity = parity ^ bitset<8>(trailer);

    if (parity.to_ulong() == 0)
    {
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
    for (int i = 0; i < payload.length(); ++i)
    {
        parity = parity ^ bitset<8>(payload[i]);
    }
    return (char)parity.to_ulong();
}
//********************************************************

//*******************BYTE STUFFING*************************
string Node::byteStuffing(string text)
{
    string payload = "";
    for (int i = 0; i < text.length(); ++i)
    {
        if (text[i] == '$')
            payload += '/';
        payload += text[i];
    }
    return '$' + payload + '$';
}
//*********************************************************

//*******************CREATE FRAME**************************
Message *Node::createFrame(string text, seq_nr seqNum)
{
    string payload = byteStuffing(text);
    char parityByte = createParity(payload, seqNum);

    Message *msg = new Message();
    msg->setMessageType(1); // 1 refers to normal message
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
    for (int i = 1; i < payload.length() - 1; ++i)
    {
        if (payload[i] == '/')
            continue;
        message += payload[i];
    }
    return message;
}
//**********************************************************

//******************Create Error****************************
void Node::modification(Message *msg)
{
    string payload(msg->getPayload());
    int size = payload.length();
    bitset<8> forerror(1);
    vector<bitset<8>> vec;
    for (int i = 0; i < size; i++)
    {
        bitset<8> xbits(payload[i]);
        vec.push_back(xbits);
    }

    int error = int(uniform(0, size));
    vec[error] = vec[error] ^ forerror;

    string collect = "";

    for (int it = 0; it < vec.size(); it++)
    {
        collect = collect + (char)(vec[it]).to_ulong();
    }

    msg->setPayload(collect.c_str());
}
//**********************************************************

//******************Receiver Function***********************
void Node::rec(Message *msg, bool isSelfMessage)
{

    if (!isSelfMessage)
    {
        if (msg->getSeqNum() == frameExpected) // if received frame is the frame expected
        {
            if (checkParity(msg)) // check for error
            {
                msg->setAckNum(frameExpected);
                inc(frameExpected);
                msg->setFrameType(ACK);
                recData.push_back(msg->getPayload());
            }
            else // if error is found send nack
            {
                msg->setAckNum(frameExpected);
                msg->setFrameType(NACK);
            }
            int Loss = int(uniform(0, 100));
            double x = (double)simTime().dbl();
            if (Loss >= LossProb) // check for loss probability
            {
                string s = to_string(msg->getAckNum());
                msg->setName(s.c_str());
                msg->setErrorType(CORRECT);
            }
            else // msg was lost
            {
                msg->setErrorType(LOSS);
            }
            scheduleAt(simTime() + processingTime, msg);
        }
        else // if the received frame wasn't the one expected, send ack for the last received frame
        {
            msg->setAckNum(dec(frameExpected));
            double x = (double)simTime().dbl();
            msg->setFrameType(ACK);
            int Loss = int(uniform(0, 100));
            if (Loss >= LossProb)
            {
                string s = to_string(msg->getAckNum());
                msg->setName(s.c_str());
                msg->setErrorType(CORRECT);
            }
            else
            {
                msg->setErrorType(LOSS);
            }
            scheduleAt(simTime() + processingTime, msg);
        }
    }
    else
    {
        double x = (double)simTime().dbl();
        logs.log_ControlFrame(to_string(x), to_string(getIndex()), msg->getFrameType(), to_string(msg->getAckNum()), msg->getErrorType());
        if (msg->getErrorType() != LOSS)
        {
            sendDelayed(msg, transmissionDelay, "out");
        }
        else
        {
            delete msg;
        }
    }
}
//**********************************************************

//*******************GO BACKN FUNCTIONS*********************
void Node::inc(seq_nr &currentSeqNum)
{
    if (currentSeqNum < maxSeqNum)
        currentSeqNum++;
    else
        currentSeqNum = 0;
}
//**********************************************************

//*******************DECREMENT******************************
seq_nr Node::dec(seq_nr lastAck)
{
    if (lastAck > 0)
        return lastAck - 1;
    else
        return maxSeqNum;
}
//**********************************************************

//*******************START TIMER****************************
void Node::startTimer(seq_nr seqNum)
{
    if (timerMessages.find(seqNum) == timerMessages.end())
    {
        Message *timerMessage = new Message("TIMEOUT");

        timerMessage->setPayload(to_string(seqNum).c_str());
        timerMessage->setMessageType(TIMEOUT_MSG);

        timerMessages[seqNum] = timerMessage;

        scheduleAt(simTime() + timeout, timerMessage);
    }
}
//**********************************************************

//*******************RESET TIMER****************************
void Node::resetTimer()
{
    for (auto it = timerMessages.begin(); it != timerMessages.end(); it++)
    {
        cancelAndDelete(it->second);
    }
    timerMessages.clear();
}
//**********************************************************

//*******************STOP TIMER*****************************
void Node::stopTimer(seq_nr seqNum)
{
    if (timerMessages[seqNum])
    {
        cancelAndDelete(timerMessages[seqNum]);
        timerMessages.erase(seqNum);
    }
}
//**********************************************************

//******************CHECK ERROR TYPE************************
ErrorType Node::checkErrorType(string errorString, Message *msg)
{
    if (errorString[1] == '1')
        return LOSS;
    if (errorString[0] == '1')
        modification(msg);
    if (errorString[2] == '1' && errorString[3] == '1')
        return DELAYED_AND_DUPLICATED;
    if (errorString[2] == '1')
        return DUPLICATED;
    if (errorString[3] == '1')
        return DELAYED;
    return CORRECT;
}
//**********************************************************

//******************HANDLE MSG ERRORS************************
void Node::handlingMsgErrors(Message *msg, ErrorType typesOfError, double currentMsg)
{
    if (typesOfError == LOSS)
    {
        msg->setErrorType(LOSS);
        scheduleAt(simTime() + processingTime * currentMsg, msg);
    }
    else if (typesOfError == DELAYED)
    {
        msg->setErrorType(DELAYED);
        scheduleAt(simTime() + processingTime * currentMsg, msg);
    }
    else if (typesOfError == DELAYED_AND_DUPLICATED)
    {
        Message *msgDup = new Message(*msg);
        msg->setErrorType(DELAYED);
        msgDup->setErrorType(DELAYED_AND_DUPLICATED);
        scheduleAt(simTime() + processingTime * currentMsg, msg);
        scheduleAt(simTime() + processingTime * currentMsg + duplicationDelay, msgDup);
    }
    else if (typesOfError == DUPLICATED)
    {
        Message *msgDup = new Message(*msg);
        msgDup->setErrorType(DUPLICATED);
        msg->setErrorType(CORRECT);
        scheduleAt(simTime() + processingTime * currentMsg, msg);
        scheduleAt(simTime() + processingTime * currentMsg + duplicationDelay, msgDup);
    }
    else if (typesOfError == CORRECT)
    {
        msg->setErrorType(CORRECT);
        scheduleAt(simTime() + processingTime * currentMsg, msg);
    }
}
//***********************************************************

//******************RESEND BUFFER****************************
void Node::resendBuffer()
{
    double currentMsg = 0.0;
    senderMsgBuffer[0]->setErrorString("0000");
    for (int i = 0; i < senderMsgBuffer.size(); ++i)
    {
        if (senderMsgBuffer[i]->getMessageState() == WAITING)
        {
            return;
        }
        //        stopTimer(senderMsgBuffer[i]->getSeqNum());
        senderMsgBuffer[i]->setMessageState(WAITING);
        Message *copyMessage = new Message(*senderMsgBuffer[i]);
        currentMsg++;
        ErrorType error = checkErrorType(senderMsgBuffer[i]->getErrorString(), copyMessage);
        handlingMsgErrors(copyMessage, error, currentMsg);
    }
}
//***********************************************************

//******************UPDATE MESSAGE STATE*********************
void Node::updateMessageStateInBuffer(seq_nr seqNum)
{
    for (int i = 0; i < senderMsgBuffer.size(); ++i)
    {
        if (senderMsgBuffer[i]->getSeqNum() == seqNum)
        {
            senderMsgBuffer[i]->setMessageState(SENT);
            return;
        }
    }
}
//***********************************************************

//******************CHECK ACK IN RANGE***********************
bool Node::isAckWithInRange(seq_nr receivedAckNum)
{ // b
    for (int i = 0; i < senderMsgBuffer.size(); ++i)
    {
        if (receivedAckNum == senderMsgBuffer[i]->getSeqNum())
            return true;
    }
    return false;
}
//***********************************************************

//******************TO PHYSICAL LAYER************************
void Node::toPhysicalLayer(Message *msg){
    string s = msg->getPayload();
    updateMessageStateInBuffer(msg->getSeqNum());

    startTimer(msg->getSeqNum());
    char duplicationIndex = msg->getErrorString()[2];
    if (msg->getErrorType() == DELAYED_AND_DUPLICATED || msg->getErrorType() == DUPLICATED)
    {
        duplicationIndex = '2';
    }
    logs.log_BeforeTransmission(to_string((double)(simTime().dbl())), to_string(myRole), to_string(msg->getSeqNum()), msg->getPayload(), msg->getTrailer(),
                                msg->getErrorString()[0], msg->getErrorString()[1] - '0', duplicationIndex, msg->getErrorString()[3]);

    if (msg->getErrorType() == DELAYED || msg->getErrorType() == DELAYED_AND_DUPLICATED)
    {
        msg->setErrorType(CORRECT);
        sendDelayed(msg, errorDelay + transmissionDelay, "out");
    }
    else if (msg->getErrorType() == CORRECT || msg->getErrorType() == DUPLICATED)
    {
        msg->setErrorType(CORRECT);
        sendDelayed(msg, transmissionDelay, "out");
    }
    else if (msg->getErrorType() == LOSS)
        delete msg;

}
//***********************************************************

//******************HANDLE ACKS******************************
void Node::handleAcks(Message *msg){
    if (isAckWithInRange(msg->getAckNum()))
    {
        int index = 0;
        for (int i = 0; i < senderMsgBuffer.size(); ++i)
        {
            index++;
            stopTimer(senderMsgBuffer[i]->getSeqNum());
            inc(frameExpected);
            if (senderMsgBuffer[i]->getSeqNum() == msg->getAckNum())
            {
                delete senderMsgBuffer[i];
                break;
            }
            delete senderMsgBuffer[i];
        }
        senderMsgBuffer.erase(senderMsgBuffer.begin(), senderMsgBuffer.begin() + index);
    }
    delete msg;
}
//***********************************************************

//******************FROM NETWORK LAYER***********************
void Node::fromNetworkLayer(Message *msg, double& currentMsg){
    msg = createFrame(data[currentDataIndex].second, nextFrameToSend);
    currentMsg++;
    msg->setMessageState(WAITING);
    // CHECK TYPES OF ERRORS
    msg->setErrorString(data[currentDataIndex].first.c_str());
    Message *newMsg = new Message(*msg);
    senderMsgBuffer.push_back(newMsg); // Put it into buffer before modification
    ErrorType typesOfError = checkErrorType(data[currentDataIndex].first, msg);
    if (currentMsg == 1)
    {
        logs.log_ReadLine(to_string((double)(simTime().dbl())), to_string(myRole), msg->getErrorString());
    }
    else
    {
        Message *printMessage = new Message(*msg);
        printMessage->setMessageType(PRINT_MSG);
        scheduleAt(simTime() + processingTime * (currentMsg - 1), printMessage);
    }
    handlingMsgErrors(msg, typesOfError, currentMsg);
}
//***********************************************************

//******************SENDER FUNCTION**************************
void Node::sender(Message *msg, bool isSelfMessage)
{
    if (isSelfMessage)
    {
        if (msg->getMessageType() == TIMEOUT_MSG)
        {
            logs.log_TimeOut(to_string((double)(simTime().dbl())), to_string(myRole), msg->getPayload());
            resetTimer();
            resendBuffer();
        }
        else if (msg->getMessageType() == PRINT_MSG)
        {
            logs.log_ReadLine(to_string((double)(simTime().dbl())), to_string(myRole), msg->getErrorString());
            delete msg;
        }
        else
            toPhysicalLayer(msg);
    }
    else
    {
        if (msg->getFrameType() == ACK)
            handleAcks(msg);
        else if (msg->getFrameType() == NACK)
            delete msg;
    }

    double currentMsg = 0.0;
    if (currentDataIndex < data.size())
    {
        while (senderMsgBuffer.size() < senderWindowSize)
        {
            if (currentDataIndex == data.size())
                break;
            fromNetworkLayer(msg, currentMsg);
            currentDataIndex++;
            inc(nextFrameToSend);
        }
    }
}
//**********************************************************

void Node::initialize()
{
    myRole = RECEIVER;
    processingTime = par("PT").doubleValue();
    LossProb = par("LP").doubleValue();
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
    frameExpected = 0;
}

void Node::handleMessage(cMessage *msg)
{
    Message *msg2 = check_and_cast<Message *>(msg);
    if (!msg->isSelfMessage())
    {
        if (msg2->getMessageType() == COORD_MSG && stoi(msg2->getPayload()) == getIndex())
        {
            coordMsg = msg;
            myRole = SENDER;
            readData();
        }
    }
    if (myRole == SENDER)
        sender(msg2, msg->isSelfMessage());
    else
        rec(msg2, msg->isSelfMessage());
}


Node::~Node()
{
    // TODO Auto-generated destructor stub
    //    if(coordMsg)
    //        delete coordMsg;
}
