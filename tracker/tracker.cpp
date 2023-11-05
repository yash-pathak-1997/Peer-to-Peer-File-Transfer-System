#include "../Utilities/Logger.h"
#include "../Utilities/Exceptions.h"
#include "../Utilities/Helpers.h"
#include "../Utilities/Headers.h"

// Instance Type
#define TYPE "Tracker"

// create_user function
bool createUserTracker(vector<string> client_input){
    string id = client_input[1];
    string pwd = client_input[2];
    if(curr_tracker->user_creds.find(id) == curr_tracker->user_creds.end()){
        curr_tracker->user_creds.insert({id, pwd});
        return true;
    }
    return false;
}

// login function
int loginUserTracker(vector<string> client_input){
    /*  1 -> login successful
        2 -> user already logged in
        3 -> incorrect credentials  */ 
    string id = client_input[1];
    string pwd = client_input[2];
    if(curr_tracker->user_creds.find(id) == curr_tracker->user_creds.end() || curr_tracker->user_creds[id] != pwd)
        return 3;
    if(curr_tracker->is_user_logged_in.find(id) == curr_tracker->is_user_logged_in.end())
        curr_tracker->is_user_logged_in.insert({id, true});
    else{
        if(curr_tracker->is_user_logged_in[id])
            return 2;
        else
            curr_tracker->is_user_logged_in[id] = true;
    }
    return 1;
}

// logout function
bool logoutUserTracker(string client_id){
    if(curr_tracker->is_user_logged_in.find(client_id) != curr_tracker->is_user_logged_in.end()){
        curr_tracker->is_user_logged_in[client_id] = false;
        return true;
    }
    return false;
}

// create_group function
bool createGroupTracker(vector<string> client_input, int client_socket, string client_id){
    string group_name = client_input[1];
    for(auto i: curr_tracker->group_list){
        if(i == group_name) 
            return false;
    }
    curr_tracker->group_admins.insert({group_name, client_id});
    curr_tracker->group_list.push_back(group_name);
    curr_tracker->group_members[group_name].insert(client_id);
    return true;
}

// list_group function
void listGroupTracker(vector<string> client_input, int client_socket){
    string msg;
    msg = "Following are the groups : ";
    if(curr_tracker->group_list.size() == 0){
        msg = GROUP_NOT_FOUND;
        write(client_socket, (void*)msg.c_str(), msg.length());
        return;
    }
    string all_groups = "";
    for(size_t i=0;i<curr_tracker->group_list.size();i++)
        all_groups += curr_tracker->group_list[i] + "|";
    cout<<all_groups<<endl;
    write(client_socket, (void*)msg.c_str(), msg.length());
    sleep(2);
    write(client_socket, &all_groups[0], all_groups.length());
}

// join_group function
void joinGroupTracker(vector<string> client_input, int client_socket, string client_id){
    string msg;
    if(curr_tracker->group_admins.find(client_input[1]) == curr_tracker->group_admins.end())
        msg = "Invalid group ID.";
    else if(curr_tracker->group_members[client_input[1]].find(client_id) == curr_tracker->group_members[client_input[1]].end()){
        curr_tracker->group_requests[client_input[1]].insert(client_id);
        msg = "Group request sent";
    }
    else
        msg = "You are already in this group";
    write(client_socket, (void*)msg.c_str(), msg.length());
}

// leave_group function
void leaveGroupTracker(vector<string> client_input, int client_socket, string client_id){
    string msg;
    if(curr_tracker->group_admins.find(client_input[1]) == curr_tracker->group_admins.end())
        msg = "Entered group id is invalid";
    else if(curr_tracker->group_members[client_input[1]].find(client_id) != curr_tracker->group_members[client_input[1]].end()){
        if(curr_tracker->group_admins[client_input[1]] == client_id)
            msg = "You are the admin of this group, you cant leave!";
        else{
            curr_tracker->group_members[client_input[1]].erase(client_id);
            msg = "Group left succesfully";
        }
    }
    else
       msg = "You are not in this group";
    write(client_socket, (void*)msg.c_str(), msg.length());
}

// list_requests function
void listReqTracker(vector<string> client_input, int client_socket, string client_id){
    string msg;
    if(curr_tracker->group_admins.find(client_input[1])==curr_tracker->group_admins.end() || curr_tracker->group_admins[client_input[1]] != client_id){
        msg = NOT_ADMIN_OF_GROUP;
        write(client_socket, (void*)msg.c_str(), msg.length());
    }
        
    else if(curr_tracker->group_requests[client_input[1]].size() == 0){
        msg = NO_PENDING_REQS;
        write(client_socket, (void*)msg.c_str(), msg.length());
    }
    else {
        msg = "Following are the pending requests : ";
        string reply = "";
        for(auto i = curr_tracker->group_requests[client_input[1]].begin();i!= curr_tracker->group_requests[client_input[1]].end();i++)
            reply += string(*i) + "|";
        write(client_socket, (void*)msg.c_str(), msg.length());
        sleep(2);
        write(client_socket, &reply[0], reply.length());
        return;
    }
}

// accept_request function
void acceptReqTracker(vector<string> client_input, int client_socket, string client_id){
    string msg;
    if(curr_tracker->group_admins.find(client_input[1]) == curr_tracker->group_admins.end())
        msg = "Entered group id is invalid";
    else if(curr_tracker->group_admins.find(client_input[1])->second == client_id){
        curr_tracker->group_requests[client_input[1]].erase(client_input[2]);
        curr_tracker->group_members[client_input[1]].insert(client_input[2]);
        msg = "Request is accepted";
    }
    else
        msg = "Request cannot be accepted. Not an admin of this group";
    write(client_socket, (void*)msg.c_str(), msg.length());
}

// list_files function
void listFilesTracker(vector<string> client_input, int client_socket, string client_id){
    string msg;
    if(curr_tracker->group_admins.find(client_input[1]) == curr_tracker->group_admins.end()){
        msg = GROUP_NOT_FOUND;
        write(client_socket, (void*)msg.c_str(), msg.length());
    }
    else if(curr_tracker->seeder_list[client_input[1]].size() == 0){
        msg = FILE_NOT_EXISTS;
        write(client_socket, (void*)msg.c_str(), msg.length());
    }
    else{
        string reply = "";
        for(auto i: curr_tracker->seeder_list[client_input[1]])
            reply += i.first + "|";
        reply = reply.substr(0, reply.length()-2);
        write(client_socket, &reply[0], reply.length());
        return;
    }
}

// upload_file function
void uploadFileTracker(vector<string> client_input, int client_socket, string client_id){
    string msg;
    if(curr_tracker->group_members.find(client_input[2]) == curr_tracker->group_members.end()){
        msg = GROUP_NOT_FOUND;
        write(client_socket, (void*)msg.c_str(), msg.length());
    }
    else if(curr_tracker->group_members[client_input[2]].find(client_id) == curr_tracker->group_members[client_input[2]].end()){
        msg = USER_NOT_IN_GROUP;
        write(client_socket, (void*)msg.c_str(), msg.length());
    }
    else if(!pathExists(client_input[1])){
        msg = FILE_NOT_EXISTS;
        write(client_socket, (void*)msg.c_str(), msg.length());
    }
    else{
        char file_info[524288];
        bzero(file_info, 524288);
        write(client_socket, "Uploading requested file .... ", 30);
        if(read(client_socket , file_info, 524288)){
            if(string(file_info) == FILE_ALREADY_UP){
                logObj.log(file_info);
                return;
            }

            /*FILE_INFO_VEC -> FILEPATH | CLIENT IP:CLIENT PORT | FILESIZE | FILE HASH | PIECEWISE HASH*/
            vector <string> file_info_vec = splitString(string(file_info), "|");
            
            // store file info
            string file_name = splitString(string(file_info_vec[0]), "/").back();
            if(curr_tracker->seeder_list[client_input[2]].find(file_name) != curr_tracker->seeder_list[client_input[2]].end())
                curr_tracker->seeder_list[client_input[2]][file_name].insert(client_id);
            else
                curr_tracker->seeder_list[client_input[2]].insert({file_name, {client_id}});
            curr_tracker->file_size[file_name] = file_info_vec[2];

            // store per piece hash
            string hash_piece = "";
            for(size_t i=4;i<file_info_vec.size();i++){
                hash_piece += file_info_vec[i];
                if(i != file_info_vec.size()-1) 
                    hash_piece += "|";
            }
            curr_tracker->hash_per_piece[file_name] = hash_piece;
            
            // write to client
            msg = UPLOAD_SUCCESS;
            write(client_socket, (void*)msg.c_str(), msg.length());
        }
        return;
    }
}

// download_file function
void downloadFileTracker(vector<string> client_input, int client_socket, string client_id){
    string msg;
    if(curr_tracker->group_members.find(client_input[1]) == curr_tracker->group_members.end()){
        msg = GROUP_NOT_FOUND;
        write(client_socket, (void*)msg.c_str(), msg.length());
        return;
    }
    else if(curr_tracker->group_members[client_input[1]].find(client_id) == curr_tracker->group_members[client_input[1]].end()){
        msg = NOT_MEMBER_GROUP;
        write(client_socket, (void*)msg.c_str(), msg.length());
        return;
    }
    else{
        if(!pathExists(client_input[3])){
            msg = FOLDER_NOT_EXISTS;
            write(client_socket, (void*)msg.c_str(), msg.length());
            return;
        }
        msg = "Downloading ....";
        write(client_socket, (void*)msg.c_str(), msg.length());

        // read file info from client
        char file_info[524288];
        bzero(file_info, 524288);
        read(client_socket , file_info, 524288);
        logObj.log("Recieved file info for download : " + string(file_info));
        
        // split file info in vector
        vector<string> file_info_vec = splitString(string(file_info), "|");
        string reply = "";
        if(curr_tracker->seeder_list[client_input[1]].find(file_info_vec[0]) != curr_tracker->seeder_list[client_input[1]].end()){
            for(auto i: curr_tracker->seeder_list[client_input[1]][file_info_vec[0]]){
                if(curr_tracker->is_user_logged_in[i])
                    reply += curr_tracker->clientIdPort[i] + "|";
            }
            reply += curr_tracker->file_size[file_info_vec[0]];
            write(client_socket, &reply[0], reply.length());
            write(client_socket, &curr_tracker->hash_per_piece[file_info_vec[0]][0], curr_tracker->hash_per_piece[file_info_vec[0]].length());
            curr_tracker->seeder_list[client_input[1]][client_input[2]].insert(client_id);
            // waiting for final message
            bzero(file_info, 524288);
            read(client_socket , file_info, 524288);
            sleep(5);
            return;
        }
    }
}

// Driver code for incoming stream from clients
void driveConnection(int client_socket){
    string client_id = "";
    string client_group_id = "";
    logObj.log("Thread started for a new Client. The socket number is : " + to_string(client_socket));

    //Busy waiting on command send by clients
    while(true){

        // fetch command from client
        char client_cmd[1024];
        bzero(client_cmd, 1024);
        read(client_socket , client_cmd, 1024);
        logObj.log("Client command : " + string(client_cmd));

        // divide command into words and push in vector
        string str, in = string(client_cmd);
        stringstream ss(in);
        vector<string> client_input;
        while(ss >> str)
            client_input.push_back(str);

        // Command - create_user
        if(client_input[0] == CREATE_USER){
            string msg;
            if(client_input.size() != 3)
               msg = ARGS_COUNT_INCORRECT;
            else{
                if(!createUserTracker(client_input))
                    msg = "Entered user exists";
                else
                    msg = "New Account is created";
            }
            write(client_socket, (void*)msg.c_str(), msg.length());
        }

        // Command - login
        else if(client_input[0] == LOGIN){
            string msg;
            if(client_input.size() != 3){
                msg = ARGS_COUNT_INCORRECT;
                write(client_socket, (void*)msg.c_str(), msg.length());
            }
            else{
                int flag = loginUserTracker(client_input);
                if(flag == 1){
                    msg = LOGIN_SUCCESS;
                    write(client_socket, (void*)msg.c_str(), msg.length());
                    client_id = client_input[1];
                    char buf[100];
                    read(client_socket, buf, 100);
                    string addr = string(buf);
                    curr_tracker->clientIdPort[client_id] = addr;
                    logObj.log("Client : ( "+addr+" ) is logged in with user : "+client_id);
                }
                else if(flag == 2){
                    msg = ALREADY_LOGGED_IN;
                    write(client_socket, (void*)msg.c_str(), msg.length());
                }
                else{
                    msg = INVALID_CREDS;
                    write(client_socket, (void*)msg.c_str(), msg.length());
                }
            }
        }

        // Command - logout
        else if(client_input[0] ==  LOGOUT){
            string msg;
            if(client_input.size() != 1){
                msg = ARGS_COUNT_INCORRECT;
                write(client_socket, (void*)msg.c_str(), msg.length());
            }
            if(logoutUserTracker(client_id))
                msg = "User successfully logged out";
            else
                msg = "This user was not logged in!";
            write(client_socket, (void*)msg.c_str(), msg.length());
        }

        // Command - create_group
        else if(client_input[0] == CREATEGROUP){
            string msg;
            if(client_input.size() != 2)
                msg = ARGS_COUNT_INCORRECT;
            else{
                if(createGroupTracker(client_input, client_socket, client_id)){
                    client_group_id = client_input[1];
                    msg = "Group created";
                }
                else
                    msg = "Group already exists";
            }
            write(client_socket, (void*)msg.c_str(), msg.length());
        }

        // Command - list_group
        else if(client_input[0] == LISTGROUP){
            string msg;
            if(client_input.size() != 1){
                msg = ARGS_COUNT_INCORRECT;
                write(client_socket, (void*)msg.c_str(), msg.length());
            }
            else
                listGroupTracker(client_input, client_socket);
        }

        // Command - join_group
        else if(client_input[0] == JOINGROUP){
            string msg;
            if(client_input.size() != 2){
                msg = ARGS_COUNT_INCORRECT;
                write(client_socket, (void*)msg.c_str(), msg.length());
            }
            else
                joinGroupTracker(client_input, client_socket, client_id);
        }

        // Command - leave_group
        else if(client_input[0] == LEAVEGROUP){
            string msg;
            if(client_input.size() != 2){
                msg = ARGS_COUNT_INCORRECT;
                write(client_socket, (void*)msg.c_str(), msg.length());
            }
            else
                leaveGroupTracker(client_input, client_socket, client_id);
        }

        // Command - list_requests
        else if(client_input[0] == LISTREQ){
            string msg;
            if(client_input.size() != 2){
                msg = ARGS_COUNT_INCORRECT;
                write(client_socket, (void*)msg.c_str(), msg.length());
            }
            else
                listReqTracker(client_input, client_socket, client_id);
        }

        // Command - accept_requests
        else if(client_input[0] == ACCEPTREQ){
            string msg;
            if(client_input.size() != 3){
                msg = ARGS_COUNT_INCORRECT;
                write(client_socket, (void*)msg.c_str(), msg.length());
            }
            else
                acceptReqTracker(client_input, client_socket, client_id);
        }

        // Command - list_files
        else if(client_input[0] == LISTFILES){
            string msg;
            if(client_input.size() != 2){
                msg = ARGS_COUNT_INCORRECT;
                write(client_socket, (void*)msg.c_str(), msg.length());
            }
            else
                listFilesTracker(client_input, client_socket, client_id);
        }

        // Command - upload_file
        else if(client_input[0] == UPLOADFILE){
            string msg;
            if(client_input.size() != 3){
                msg = ARGS_COUNT_INCORRECT;
                write(client_socket, (void*)msg.c_str(), msg.length());
            }
            else
                uploadFileTracker(client_input, client_socket, client_id);
        }

        // Command - download_file
        else if(client_input[0] == DOWNLOADFILE){
            string msg;
            if(client_input.size() != 4){
                msg = ARGS_COUNT_INCORRECT;
                write(client_socket, (void*)msg.c_str(), msg.length());
            }
            else
                downloadFileTracker(client_input, client_socket, client_id);
        }
        
        // Invalid Command
        else{
            string msg = "Inavlid command entered";
            write(client_socket, (void*)msg.c_str(), msg.length());
        }
    }

    logObj.log("Thread ended for client with socket number : " + to_string(client_socket));
    close(client_socket);
}

int main(int argc, char* argv[]){

    // Check if the arguments are correct
    try{
        if(argc != 3)
            throw ArgumentsException();
    }
    catch(ArgumentsException& e){
        cout<<e.printMessage(TYPE);
        return 0;
    }

    // Process the arguments
    vector <Tracker*> trackers;
    curr_tracker = processRuntimeArgs(argc, argv, trackers, TYPE);

    // Set log file for current tracker
    logObj = Logger("../logs", TYPE, argv[2]);
    
    // Create socket for tracker
    int tracker_socket;
    struct sockaddr_in tracker_address;
    int opt_val = 1;
    int addr_len = sizeof(tracker_address);
    pthread_t  exitDetectionThreadId;
    // --Step1 : socket()
    if((tracker_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Socket creation failed");
        return -1;
    }
    // -- Step2 : setsockopt()
    if(setsockopt(tracker_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt_val, sizeof(opt_val))){
        perror("Error in setsockopt");
        return -1;
    }
    // -- Step3 : bind()
    tracker_address.sin_family = AF_INET;
    tracker_address.sin_port = htons(stoi(curr_tracker->curr_port));
    if(inet_pton(AF_INET, curr_tracker->curr_ip.c_str(), &tracker_address.sin_addr)<=0){
        cout<<"Address not valid!";
        return -1;
    }
    if(bind(tracker_socket, (struct sockaddr *)&tracker_address,  sizeof(tracker_address))<0){
        perror("bind failed");
        return -1;
    }

    logObj.log("Tracker socket is created");

    // Make the tracker listen to incoming connections
    if(listen(tracker_socket, 5) < 0){
        perror("listen");
        return -1;
    }
    logObj.log("Tracker is now listening at "+curr_tracker->curr_addr+" ...");

    // Create a thread for Tracker and it will end when "quit" is entered
    if(pthread_create(&exitDetectionThreadId, NULL, checkQuit, (void*)NULL) == -1){
        perror("Issue in thread creation for Tracker. Exiting ...");
        return -1;
    }

    // Accept incoming connections and push them in thread vector
    vector<thread> thread_vector;
    while(true){
        int client_socket;
        if((client_socket = accept(tracker_socket, (struct sockaddr *)&tracker_address, (socklen_t *)&addr_len)) < 0){
            perror("Incoming connecton error");
            logObj.log("Error in Incoming connecton");
        }
        logObj.log("Connection Accepted");
        thread_vector.push_back(thread(driveConnection, client_socket));
    }
    for(auto itr=thread_vector.begin();itr!=thread_vector.end();itr++){
        if(itr->joinable() == true) 
            itr->join();
    }

    // Exit Tracker
    logObj.log("Quiting Tracker");
    close(tracker_socket);
    return 0;
}