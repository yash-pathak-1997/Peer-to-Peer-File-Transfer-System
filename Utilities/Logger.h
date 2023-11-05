#include <iostream>
#include <time.h>
#include <ctime>
#include <fstream>
using namespace std;

// 4 General levels of Logging
enum level {
    WARN,
    INFO,
    ERROR,
    DEBUG
};

class Logger {

    public:
        string logFilePath;

        Logger(){}  // default constructor

        Logger(string loggingPath, string type, string num){
            this->logFilePath = loggingPath + "/" + type + "_log_"+getDateTime()+".txt";
        }

        string getDateTime(){
            time_t now = time(0);
            char  buf[80];
            struct tm time_struct;
            time_struct = *localtime(&now);
            strftime(buf, sizeof(buf), "%Y-%m-%d %X", &time_struct);
            return string(buf);
        }

        void log(string msg, bool printToConsole = true){
            string currentTS = getDateTime();
            ofstream ofst(this->logFilePath.c_str(), std::ios_base::out | std::ios_base::app);
            ofst << currentTS << '\t' << msg << '\n';
            if(printToConsole)
                cout << currentTS << '\t' << msg << '\n';
            ofst.close();
        }
}logObj;

extern Logger logObj;
