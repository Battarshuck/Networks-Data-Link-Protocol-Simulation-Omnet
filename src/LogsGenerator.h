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

class LogsGenerator {

private:
    ofstream logFile;
public:
    LogsGenerator();
    virtual ~LogsGenerator();

    void log_ReadLine(string, string, string);
    void log_BeforeTransmission(string startTime, string id, string seqNum , string payload, char trailer,
            string modified, int lost, string duplicate, string delay);
    void log_TimeOut(string, string, string);
    void log_ControlFrame(string, string, string, string, int);
};

#endif /* LOGSGENERATOR_H_ */
