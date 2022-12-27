/*
 * LogsGenerator.cpp
 *
 *  Created on: Dec 21, 2022
 *      Author: mosmo
 */

#include "LogsGenerator.h"
#include <bitset>

LogsGenerator::LogsGenerator() {
    // TODO Auto-generated constructor stub
//    logFile.open("output.txt");
    logFile = new ofstream("output.txt");
}

LogsGenerator::~LogsGenerator() {
    // TODO Auto-generated destructor stub
    logFile->close();
}

void LogsGenerator::log_ReadLine(string startTime, string id, string errorCode)
{
    *logFile << "At time " << startTime <<", Node["<<id<<"] Introducing channel error with code = [" << errorCode <<"]." << endl;
}

void LogsGenerator::log_BeforeTransmission(string startTime, string id, string seqNum , string payload, char trailer,
        char modified, int lost, char duplicate, char delay)
{
    bitset<8> temp(trailer);
    string myLost;
    string isModified = "1";
    if(lost == 0)
        myLost = "No";
    else
        myLost = "Yes";
    if(modified == '0')
        isModified = "-1";


    *logFile << "At time "<<startTime<<" Node["<<id<<"] [sent] frame with seq_Num["<<seqNum<<"] and Payload["<<payload<<"]"
            <<"and Trailer["<<temp.to_string()<<"] Modified ["<< isModified <<"], Lost ["<<myLost<<"] Duplicate ["<<duplicate<<"] Delay ["
            << delay << "]" << endl;
}

void LogsGenerator::log_TimeOut(string startTime, string id, string seqNum)
{
    *logFile << "Time out event at time "<<startTime<<", at Node["<<id<<"] for frame with seq_num = "<<seqNum<<endl;
}

void LogsGenerator::log_ControlFrame(string startTime, string id, int type, string seqNum, int loss)
{
    string myLoss;
    string ackType;
        if(loss == 0)
            myLoss = "No";
        else
            myLoss = "Yes";

        if(type==1)
            ackType="ACK";
        else
            ackType="NACK";

    *logFile << "At time "<< startTime <<", Node["<<id<<"] sending "<< ackType << " with number["<<seqNum<<"], loss ["<< myLoss <<"]"<<endl;
}



