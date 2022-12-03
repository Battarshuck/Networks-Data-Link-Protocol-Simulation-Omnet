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

#include "coordinator.h"
using namespace std;


Define_Module(Coordinator);


void Coordinator::initialize()
{
    // TODO - Generated method body
    ifstream newfile;
    newfile.open("coor.txt");

    Message* msg = new Message("Hi");
    string startingNode, startingTime;
    newfile >> startingNode;
    msg->setPayload(startingNode.c_str());
    newfile >> startingTime;
    scheduleAt(simTime()+stod(startingTime), msg);
    newfile.close();

}

void Coordinator::handleMessage(cMessage *msg)
{
    Message* msg2 = check_and_cast<Message*>(msg);
    msg2->setMessageType(COORD_MSG);
    send(msg, "out", stoi(msg2->getPayload()));
    cout << "----------------------------" << endl;
    cout << "Sending from coordinator to node "<< stoi(msg2->getPayload()) << " at " << (double)(simTime().dbl()) << endl;
    cout << "----------------------------" << endl;
    // TODO - Generated method body
}
