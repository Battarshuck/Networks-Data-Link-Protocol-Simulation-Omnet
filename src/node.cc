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
#include <bitset>
using namespace std;

Define_Module(Node);

//*******************READING DATA*********************
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


//*******************FRAME CREATION*********************
char Node::createParity(string payload, int seqNum)
{

    bitset<8> parity(seqNum);
    for(int i = 0; i < payload.length(); ++i)
    {
        parity = parity ^ bitset<8>(payload[i]);
    }

    cout << parity.to_string()<<endl;
    return (char)parity.to_ulong();

}

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

Message* Node::createFrame(string text, int seqNum)
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

Message* Node::modification(Message*msg)
{
    string payload(msg->getPayload());
    //cout<<"Payload Before modification: "<<payload<<endl;
    int size=payload.length();
    bitset<8> forerror (1);
    vector<bitset<8>> vec;
    for(int i=0;i<size;i++)
    {
        bitset<8> xbits (payload[i]);
        vec.push_back(xbits);
        //cout<<xbits.to_string()<<endl;
    }

    int error=int(uniform(0,size));
       vec[error]=vec[error]^forerror;

    string collect="";

    for(int it=0;it<vec.size();it++)
    {
        collect=collect+(char)(vec[it]).to_ulong();
    }
    //cout<<"New payload is: "<<collect<<endl;
    //cout<<"Payload Size: "<<size<<endl;
    msg->setPayload(collect.c_str());
    return msg;
    }
//*******************************************************


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
//**********************************************************

void Node::initialize()
{
    // TODO - Generated method body
    myRole = RECEIVER;
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
    }
    else{
        cout << "----------------------------" << endl;
        cout<< "Sending at " << (double)(simTime().dbl()) << endl;
        cout << "----------------------------" << endl;
        msg2->setMessageType(NORMAL_MSG);
        if(myRole == SENDER){

            //**************************TEST***************************
            string text = data[0].second;
            msg2 = createFrame(text, 1);
            msg2=modification(msg2);
            cout<< msg2->getSeqNum()<<" "<<msg2->getPayload() << " "<<msg2->getTrailer()<<endl;
            //******************************************************

            msg2->setName("My role is a sender");
        }
        else
        {

            //**********************TEST*******************
            if(checkParity(msg2))
                cout << "correct" << endl;
            else
                cout << "error detected"<<endl;
            //****************************************

            msg2->setName("My role is a receiver");
        }
        send(msg2, "out");
    }
}
