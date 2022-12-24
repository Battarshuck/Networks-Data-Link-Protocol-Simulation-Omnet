/*
 * LogsGenerator.h
 *
 *  Created on: Dec 21, 2022
 *      Author: mosmo
 */
#include <iostream>
#include <fstream>
#include <string>
#ifndef LOGSGENERATOR_H_
#define LOGSGENERATOR_H_

using namespace std;

static ofstream* logFile;
class LogsGenerator {

private:


public:
    LogsGenerator();
    virtual ~LogsGenerator();

    void log_ReadLine(string, string, string);
    void log_BeforeTransmission(string startTime, string id, string seqNum , string payload, char trailer,
            char modified, int lost, char duplicate, char delay);
    void log_TimeOut(string, string, string);
    void log_ControlFrame(string, string, string, string, int);
};

#endif /* LOGSGENERATOR_H_ */
