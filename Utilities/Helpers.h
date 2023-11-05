#include <bits/stdc++.h>
#include <iostream>
#include "Models.h"
#include <sys/stat.h>
#define ll long long int

using namespace std;

// Split string for IP and PORT
pair <string, string> splitStringIpPort(string info, string delim = ":"){
    string ip = info.substr(0, info.find(delim));
    string port = info.substr(info.find(delim)+1, 5);
    return make_pair(ip, port);
}

// General Split string
vector<string> splitString(string str, string delim){
    size_t pos = 0;
    vector<string> res;
    while ((pos = str.find(delim)) != string::npos) {
        string t = str.substr(0, pos);
        res.push_back(t);
        str.erase(0, pos + delim.length());
    }
    res.push_back(str);
    return res;
}

// Process runtime arguments to fetch data
Tracker* processRuntimeArgs(int argc, char *argv[], vector <Tracker*>& trackers, string type){
    if(type == "Tracker"){
        pair <string,string> tracker_info;
        Tracker* curr_tracker;
        string tracker_info_file_path = argv[1];
        FILE * tracker_info_file_pointer = fopen(tracker_info_file_path.c_str(), "right");
        
        // check if file exsists
        try{
            if (tracker_info_file_pointer == NULL)
                throw InfoFileNotExistsException();
        }
        catch(InfoFileNotExistsException& e){
            cout << e.printMessage(type) << '\n';
            return 0;
        }
        
        // fetch line by line the Tracker details
        char* info = NULL;
        size_t len = 0;
        while ((getline(&info, &len, tracker_info_file_pointer)) != -1) {
            tracker_info = splitStringIpPort(info);
            Tracker* t = new Tracker(tracker_info.first,tracker_info.second,tracker_info.first,tracker_info.second);
            trackers.push_back(t);
        }
        
        // choose current tracker based on argument 3
        if(argv[2] == (string)"1")
            curr_tracker = trackers[0];
        else   
            curr_tracker = trackers[1];
        
        // close file and free memory
        fclose(tracker_info_file_pointer);
        if (info)
            free(info);
        
        return curr_tracker;
    }
    else{
        char* client_info_arg = argv[1];
        string tracker_info_file_path = argv[2];
        pair <string,string> tracker_info;
        vector <string> client_info;
        Tracker* curr_tracker;
        FILE * tracker_info_file_pointer = fopen(tracker_info_file_path.c_str(), "right");
        
        // check if file exsists
        try{
            if (tracker_info_file_pointer == NULL)
                throw InfoFileNotExistsException();
        }
        catch(InfoFileNotExistsException& e){
            cout << e.printMessage(type) << '\n';
            return 0;
        }

        // fetch line by line the Tracker details
        char* info = NULL;
        size_t len = 0;
        while ((getline(&info, &len, tracker_info_file_pointer)) != -1) {
            tracker_info = splitStringIpPort(info);
            Tracker* t = new Tracker(tracker_info.first,tracker_info.second,tracker_info.first,tracker_info.second);
            trackers.push_back(t);
        }
        curr_tracker = trackers[0];

        // close file and free memory
        fclose(tracker_info_file_pointer);
        if (info)
            free(info);

        // fetch client info
        client_info = splitString(client_info_arg, ":");
        Client* temp = new Client(client_info[0], client_info[1], string(client_info[0]+":"+client_info[1]));
        curr_client = temp;
        return curr_tracker;
    }
}

// Check if quit is entered for tracker
void* checkQuit(void* args){
    while(true){
        string input;
        getline(cin, input);
        if(input == "quit")
            exit(0);
    }
}

// Check if path exists
bool pathExists(const string &path){
    struct stat file_buffer;
    if(stat(path.c_str(), &file_buffer) == 0)
        return true;
    return false;
}

// Get file size from given path (provided file exists)
long long getFileSize(char *path){
    FILE *fp = fopen(path, "rb"); 
    ll size=-1;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp)+1;
    fclose(fp);
    return size;
}

void setPieceVector(string file_name, ll left, ll right, bool is_upload){
    if(is_upload){
        vector<int> tmp(right-left+1, 1);
        curr_client->file_piece_info[file_name] = tmp;
    }
    else{
        curr_client->file_piece_info[file_name][left] = 1;
        logObj.log("Piece vector is updated for - " + file_name + " at " + to_string(left));
    }
}