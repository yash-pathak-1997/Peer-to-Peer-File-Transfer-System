#include "../Utilities/Logger.h"
#include "../Utilities/Exceptions.h"
#include "../Utilities/Helpers.h"
#include "../Utilities/Headers.h"

//Instance Type
#define TYPE "Client"

void getStringHash(string piece_str, string& hash){
    unsigned char md[20];
    SHA1(reinterpret_cast<const unsigned char *>(&piece_str[0]), piece_str.length(), md);  // SHA1 hash of piece_str
    for(int i=0;i<20;i++){
        char buf[3];
        sprintf(buf, "%02x", md[i]&0xff);
        hash += string(buf);
    }
    hash += "|";
}

string getHash(char* path){
    // open file to get string hash
    int  i, sz;
    FILE *fp_stream;
    long long file_size = getFileSize(path);
    int total_pieces = file_size/PIECE_SIZE + 1;
    char line[LINE_SIZE + 1];
    string hash = "";
    fp_stream = fopen(path, "r");

    // get piece-wise | seperated hash
    for(i=0;i<total_pieces;i++){
        sz = 0;
        string piece_str;
        int eol;
        while(sz < PIECE_SIZE && (eol = fread(line, 1, min(LINE_SIZE-1, PIECE_SIZE-sz), fp_stream))){
            line[eol] = '\0';
            piece_str += line;
            bzero(line, sizeof(line));
            sz += strlen(line);
        }
        getStringHash(piece_str, hash);  // updates 'hash' with | seperated piece wise hash
    }

    fclose(fp_stream);
    hash.pop_back();hash.pop_back();  // remove '\0' and '|' in the end
    return hash;
}

string getFileHash(char* path){
    // read file in a string
    ostringstream buf;
    ifstream input(path);
    buf<<input.rdbuf();
    string file_str =  buf.str(), hash;
    
    // SHA256 of entire file
    unsigned char md[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(&file_str[0]), file_str.length(), md);
    for(int i=0;i<SHA256_DIGEST_LENGTH;i++){
        char buf[3];
        sprintf(buf, "%02x", md[i]&0xff);
        hash += string(buf);
    }

    return hash;
}

void sendPieceClient(char* file_path, int piece_no, int client_socket){
    // open file and set pointer
    ifstream fp_stream(file_path, ios::in|ios::binary);
    fp_stream.seekg(piece_no*PIECE_SIZE, fp_stream.beg);
    logObj.log("Sending file data which starts at : " + to_string(fp_stream.tellg()));

    // read data from file and send
    char buffer[PIECE_SIZE];
    bzero(buffer, PIECE_SIZE);   
    fp_stream.read(buffer, sizeof(buffer));
    
    // send to client peer again
    send(client_socket, buffer, fp_stream.gcount(), 0);
    logObj.log("It was sent till : "+to_string(fp_stream.tellg()));
    
    // close file
    fp_stream.close();
}

int writePieceClient(int peer_socket, ll piece_no, char* file_path){
    int n, total = 0;
    char buffer[PIECE_SIZE];
    string content = "";
    
    // write file content (piece wise)
    while(total < PIECE_SIZE){
        n = read(peer_socket, buffer, PIECE_SIZE-1);
        if(n <= 0)
            break;
        
        buffer[n] = 0;
        fstream outfile(file_path, fstream::in | fstream::out | fstream::binary);
        outfile.seekp(piece_no*PIECE_SIZE+total, ios::beg);
        outfile.write(buffer, n);
        outfile.close();

        logObj.log("Piece written from : "+ to_string(piece_no*PIECE_SIZE + total));
        logObj.log("Piece written till : " + to_string(piece_no*PIECE_SIZE + total + n-1));

        content += buffer;total += n;
        bzero(buffer, PIECE_SIZE);
    }
    
    string hash = "";
    getStringHash(content, hash);
    hash.pop_back();hash.pop_back();
    
    string file_name = splitString(string(file_path), "/").back();
    setPieceVector(file_name, piece_no, piece_no, false);

    return 0;
}

string peerConnection(char* server_peer_ip, char* server_peer_port, string command){
    int peer_socket = 0;
    struct sockaddr_in peer_serv_addr;
    string current_task = splitString(command, "|").front();

    // socket creation
    if((peer_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){  
        logObj.log("Socket creation error in peerConnection");
        return "err";
    }
    logObj.log("Socket Created in peerConnection");

    // connect to peer
    peer_serv_addr.sin_family = AF_INET;
    peer_serv_addr.sin_port = htons(stoi(string(server_peer_port)));
    logObj.log("Needs to connect to " + string(server_peer_ip) + ":" + string(server_peer_port));
    if(inet_pton(AF_INET, server_peer_ip, &peer_serv_addr.sin_addr) < 0){
        logObj.log("Peer address not valid!");
        return "err";
    }
    if(connect(peer_socket, (struct sockaddr *)&peer_serv_addr, sizeof(peer_serv_addr)) < 0){ 
        logObj.log("Error while connecting to peer!");
        return "err";
    }
    logObj.log("Connected to peer " + string(server_peer_ip) + ":" + string(server_peer_port));
 
    // task - get_piece_list
    if(current_task == "get_piece_list"){
        // send current task to peer
        send(peer_socket, &command[0], strlen(&command[0]), MSG_NOSIGNAL);
        logObj.log("Command is sent to peer : " + command);
        // read reply from peer
        char server_reply[10240];
        bzero(server_reply, 10240);
        read(peer_socket, server_reply, 10240);
        logObj.log("Got Reply : " + string(server_reply));
        // close peer socket
        close(peer_socket);
        return string(server_reply);
    }

    // task - get_piece
    else if(current_task == "get_piece"){
        // send reply to peer
        send(peer_socket, &command[0], strlen(&command[0]), MSG_NOSIGNAL);
        logObj.log("Task is sent to peer : " + command);
        // getting piece items
        vector <string> cmd_items = splitString(command, "|");
        string dest_path = cmd_items[3];
        ll piece_no = stoll(cmd_items[2]);
        logObj.log("Getting piece " + to_string(piece_no) + " from "+ string(server_peer_port));
        writePieceClient(peer_socket, piece_no, &dest_path[0]);
        return "got piece";
    }

    // task - get_file_path
    else if(current_task == "get_file_path"){
        // send reply to peer
        send(peer_socket , &command[0] , strlen(&command[0]) , MSG_NOSIGNAL );
        char server_reply[10240];
        // read reply from peer
        bzero(server_reply, 10240);
        read(peer_socket, server_reply, 10240);
        logObj.log("Server reply for get file path : " + string(server_reply));
        curr_client->file_to_filepath[splitString(command, "|").back()] = string(server_reply);
    }

    close(peer_socket);
    logObj.log("Terminating connection with -" + string(server_peer_ip) + ":" + string(server_peer_port));
    return "done";
}

void getPieceInfo(FileDetails* fd){
    // connect to peer to get piece information
    logObj.log("Getting Piece info of : "+ fd->file_name + " from " + fd->peer_ip);
    vector <string> serverPeerAddress = splitString(string(fd->peer_ip), ":");
    string command = "get_piece_list|" + string(fd->file_name);
    string response = peerConnection(&serverPeerAddress[0][0], &serverPeerAddress[1][0], command);

    // store piece information against peer_ip
    for(size_t i=0;i<curr_client->file_pieces_peers.size();i++){
        if(response[i] == '1')
            curr_client->file_pieces_peers[i].push_back(string(fd->peer_ip));
    }

    delete fd;
}

void getPiece(PieceDetails* reqdPiece){
    // get piece details from peer
    string file_name = reqdPiece->file_name;
    vector <string> peer_ip = splitString(reqdPiece->peer_ip, ":");
    ll piece_no = reqdPiece->piece_no;
    string dest = reqdPiece->dest;
    string command = "get_piece|" + file_name + "|" + to_string(piece_no) + "|" + dest;

    logObj.log("Piece fetching details : " + reqdPiece->file_name + " " + reqdPiece->peer_ip + " " + to_string(reqdPiece->piece_no));
    peerConnection(&peer_ip[0][0], &peer_ip[1][0], command);
    
    delete reqdPiece;
    return;
}
 
void pieceSelection(vector <string> curr_input, vector <string> peers){
    logObj.log("Starting Piecewise Algorithm for file transfer ....");
    
    // current input : (command group_id file_name destination)
    ll file_size = stoll(peers.back());
    peers.pop_back();
    ll total_pieces = file_size/PIECE_SIZE + 1;
    curr_client->file_pieces_peers.clear();
    curr_client->file_pieces_peers.resize(total_pieces);

    // threads to fetch piece info from peers
    vector <thread> init_threads;
    for(size_t i=0;i<peers.size();i++){
        FileDetails* fd = new FileDetails(peers[i], curr_input[2], total_pieces);
        init_threads.push_back(thread(getPieceInfo, fd));
    }
    for(auto it=init_threads.begin();it!=init_threads.end();it++){
        if(it->joinable()) 
            it->join();
    }
    init_threads.clear();

    // check if file already exists
    string des_path = curr_input[3] + "/" + curr_input[2];
    FILE* fp = fopen(&des_path[0], "r+");
	if(fp != 0){
		logObj.log("The file to be downloaded already exists at destination! Please try again");
        fclose(fp);
        return;
	}
    string ss(file_size, '\0');
    fstream in(&des_path[0], ios::out|ios::binary);
    in.write(ss.c_str(), strlen(ss.c_str()));
    in.close();

    ll pieces_recv = 0;
    curr_client->file_piece_info[curr_input[2]].resize(total_pieces,0);
    vector<int> buf_vec(total_pieces, 0);
    curr_client->file_piece_info[curr_input[2]] = buf_vec;
    string peer_get_filepath;

    srand((unsigned) time(0));
    vector <thread> final_threads;
    while(pieces_recv < total_pieces){
        logObj.log("Getting piece no " + to_string(pieces_recv) + " out of total " + to_string(total_pieces));
        ll rand_piece;
        
        // Step1 : Select random piece no. and random peer
        while(true){
            rand_piece = rand() % total_pieces;
            logObj.log("Random piece : " + to_string(rand_piece));
            if(curr_client->file_piece_info[curr_input[2]][rand_piece] == 0)
                break;
        }
        ll available_peers = curr_client->file_pieces_peers[rand_piece].size();
        string rand_peer = curr_client->file_pieces_peers[rand_piece][rand() % available_peers];
        logObj.log("Random peer : " + rand_peer);

        // Step2 : Starting thread for that piece with peer and fetch
        PieceDetails* req = new PieceDetails(rand_piece, rand_peer, curr_input[2], curr_input[3] + "/" + curr_input[2]);
        logObj.log("Starting thread for Piece number : "+ to_string(req->piece_no));
        curr_client->file_piece_info[curr_input[2]][rand_piece] = 1;
        final_threads.push_back(thread(getPiece, req));
        pieces_recv++;
        peer_get_filepath = rand_peer;
    }

    for(auto it=final_threads.begin();it!=final_threads.end();it++){
        if(it->joinable())
            it->join();
    }

    sleep(2);
    logObj.log("Download of the file is complete!");
    curr_client->downloadedFiles.insert({curr_input[2], curr_input[1]});
    vector <string> serverAddress = splitString(peer_get_filepath, ":");
    peerConnection(&serverAddress[0][0], &serverAddress[1][0], "get_file_path|" + curr_input[2]);
    return;
}
 
int downloadFileClient(vector <string> curr_input, int client_socket){
    // curr_input -  download_file​ <group_id> <file_name> <destination_path>
    // file_details = [file_name, destination_folder, group_id]
    string file_details = "";
    file_details += curr_input[2] + "|";
    file_details += curr_input[3] + "|";
    file_details += curr_input[1];
    
    // Send file details to tracker
    logObj.log("Sending file details for download to tracker : " + file_details);
    send(client_socket , &file_details[0] , strlen(&file_details[0]) , MSG_NOSIGNAL);
    
    // Read reply-1 from tracker
    char server_reply[524288];
    bzero(server_reply, 524288);
    read(client_socket , server_reply, 524288);
    logObj.log(server_reply);

    // Read reply-2 from tracker
    vector <string> peersWithFile = splitString(server_reply, "|");
    bzero(server_reply, 524288);
    read(client_socket , server_reply, 524288);
    vector <string> buf_vec = splitString(string(server_reply), "|");
    curr_client->piece_wise_hash = buf_vec;

    // Run the piece selection algorithm and send success to tracker
    pieceSelection(curr_input, peersWithFile);
    string final_msg = "Download is successful";
    send(client_socket , &final_msg[0] , strlen(&final_msg[0]) , MSG_NOSIGNAL);

    return 0;
}

void uploadFileClient(vector <string> curr_input, int server_socket){
    // fetch filename and filepath
    string file_details = "";
    char* file_path = &curr_input[1][0];
    string file_name = splitString(string(file_path), "/").back();

    // check if file already uploaded
    if(curr_client->is_uploaded[curr_input[2]].find(file_name) != curr_client->is_uploaded[curr_input[2]].end()){
        string msg = FILE_ALREADY_UP;
        write(server_socket, &msg[0], msg.length());
        return;
    }
    else{
        curr_client->is_uploaded[curr_input[2]][file_name] = true;
        curr_client->file_to_filepath[file_name] = string(file_path);
    }

    // sending file details to tracker for upload
    string piece_hash = getHash(file_path);
    string file_hash = getFileHash(file_path);
    string file_size = to_string(getFileSize(file_path));
    file_details += string(file_path) + "|" + curr_client->addr + "|" + file_size + "|";
    file_details += file_hash + "|" + piece_hash;
    logObj.log("Sending file details to server (Upload file) : " + file_details);
    send(server_socket, &file_details[0], strlen(&file_details[0]), MSG_NOSIGNAL);
    
    // reply from tracker
    char server_reply[10240];
    bzero(server_reply, 10240);
    read(server_socket , server_reply, 10240);
    logObj.log("Server reply (Upload file) : " + string(server_reply));

    setPieceVector(file_name, 0, stoll(file_size)/PIECE_SIZE + 1, true);
}

int listGroupClient(int server_socket, string server_reply){
    char reply[40960];
    bzero(reply, 40960);
    read(server_socket, reply, 40960);
    
    // print logs
    logObj.log("Groups available : " + string(reply));

    // print screen
    cout<<server_reply<<endl;
    vector <string> groups = splitString(string(reply), "|");
    for(size_t i=0;i<groups.size()-1;i++)
        cout<<groups[i]<<endl;
    
    return 0;
}

int listReqClient(int server_socket, string server_reply){
    char reply[40960];
    bzero(reply, 40960);
    read(server_socket, reply, 40960);
    
    // print logs
    logObj.log("Requests pending : " + string(reply), false);
    
    // print screen
    cout<<server_reply<<endl;
    vector <string> requests = splitString(string(reply), "|");
    for(size_t i=0;i<requests.size()-1;i++)
        cout << requests[i] << endl;
    
    return 0;
}

void listFilesClient(int server_socket){
    char buf[1024];
    bzero(buf, 1024);
    read(server_socket, buf, 1024);
    vector <string> listOfFiles = splitString(string(buf), "|");
    for(auto i: listOfFiles)
        cout<<i<<endl;
}

int commandHandler(vector <string> curr_input, int server_socket){
    char server_reply[40960];
    bzero(server_reply, 40960);
    read(server_socket , server_reply, 40960);
 
    // First check (corner) - if arg incorrect, break immediately
    if(string(server_reply) == ARGS_COUNT_INCORRECT){
        logObj.log(server_reply);
        return 0;
    }

    logObj.log(server_reply);

    // Command -> create_user
    if(curr_input[0] == CREATE_USER){
        logObj.log(server_reply);
    }

    // Command -> login
    if(curr_input[0] == LOGIN){
        if(string(server_reply) == LOGIN_SUCCESS){
            curr_client->is_logged_in = true;
            string client_address = curr_client->addr;
            write(server_socket, &client_address[0], client_address.length());
        }
        else
            logObj.log(server_reply);
    }

    // Command -> logout
    else if(curr_input[0] == LOGOUT){
        logObj.log(server_reply);
        curr_client->is_logged_in = false;
    }

    // Command -> create_group
    else if(curr_input[0] == CREATEGROUP){
        logObj.log(server_reply);
    }

    // Command -> list_group
    else if(curr_input[0] == LISTGROUP){
        if(string(server_reply) == GROUP_NOT_FOUND)
            logObj.log(server_reply);
        else
            listGroupClient(server_socket, string(server_reply));
    }
    
    // Command -> join_group
    else if(curr_input[0] == JOINGROUP){
        logObj.log(server_reply);
    }

    // Command -> leave_group
    else if(curr_input[0] == LEAVEGROUP){
        logObj.log(server_reply);
    }

    // Command -> list_requests
    else if(curr_input[0] == LISTREQ){
        if(string(server_reply) == NOT_ADMIN_OF_GROUP || string(server_reply) == NO_PENDING_REQS)
            logObj.log(server_reply);
        else
            listReqClient(server_socket, string(server_reply));
    }
    
    // Command -> accept_request
    else if(curr_input[0] == ACCEPTREQ){
        logObj.log(server_reply);
    }
    
    // Command -> list_files
    else if(curr_input[0] == LISTFILES){
        listFilesClient(server_socket);
    }
    
    // Command -> upload_file
    else if(curr_input[0] == UPLOADFILE){
        if(string(server_reply) == USER_NOT_IN_GROUP || string(server_reply) == GROUP_NOT_FOUND || string(server_reply) == FILE_NOT_EXISTS)
            logObj.log(server_reply);
        else 
            uploadFileClient(curr_input, server_socket);
    }

    // Command -> download_file
    else if(curr_input[0] == DOWNLOADFILE){
        if(string(server_reply) == GROUP_NOT_FOUND || string(server_reply) == NOT_MEMBER_GROUP || string(server_reply) == FOLDER_NOT_EXISTS)
            logObj.log(server_reply);
        else if(curr_client->downloadedFiles.find(curr_input[2])!= curr_client->downloadedFiles.end())
            logObj.log("File is already downloaded! Please check");
        else 
            downloadFileClient(curr_input, server_socket);
    }

    return 0;
}

void driveClientReq(int client_socket){
    // read client input
    char input_s[4096];
    bzero(input_s, 4096);
    read(client_socket , input_s, 4096);
    vector <string> client_input = splitString(string(input_s), "|");

    // print logs
    logObj.log("Client Socket no : " + to_string(client_socket));
    logObj.log("Client request at server : " + string(input_s));
    logObj.log(client_input[0]);

    // client command - get_piece_list
    if(client_input[0] == "get_piece_list"){
        logObj.log("Sending piece vector ....");
        string filename = client_input[1];
        vector<int> piece_vector = curr_client->file_piece_info[filename];
        string buf_vec = "";
        for(auto i: piece_vector) 
            buf_vec += to_string(i);
        char* reply = &buf_vec[0];
        write(client_socket, reply, strlen(reply));
        logObj.log("Sent : " + string(reply));
    }

    // client_command - get_piece
    else if(client_input[0] == "get_piece"){
        //client_input = [get_piece, filename, to_string(piece_no), destination]
        logObj.log("Sending piece ....");
        string filepath = curr_client->file_to_filepath[client_input[1]];
        ll piece_no = stoll(client_input[2]);
        logObj.log("File Path : "+ filepath);
        logObj.log("Sending " + to_string(piece_no) + " from " + string(curr_client->ip) + ":" + curr_client->port);
        sendPieceClient(&filepath[0], piece_no, client_socket);
    }

    // client_command - get_file_path
    else if(client_input[0] == "get_file_path"){
        string filepath = curr_client->file_to_filepath[client_input[1]];
        logObj.log("Command from peer client : " +  string(input_s));
        write(client_socket, &filepath[0], strlen(filepath.c_str()));
    }

    // close client
    close(client_socket);
    return;
}

void* peerAsServer(void* arg){
    struct sockaddr_in server_address;
    int addr_len = sizeof(server_address);
    int opt_val = 1;
    int server_socket;

    // --Step1 : socket()
    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("socket failed");
        exit(1);
    }
    logObj.log("(As server) Client socket creation successful!");

    // --Step2 : setsockopt()
    if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt_val, sizeof(opt_val))){
        perror("setsockopt");
        exit(1);
    }
    logObj.log("(As server) Setsockopt is successful!");

    // --Step3 : inet_pton()
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(stoi(curr_client->port));
    if(inet_pton(AF_INET, &curr_client->ip[0], &server_address.sin_addr)<=0){ 
        logObj.log("(As server) Invalid client address!");
        return NULL;
    }

    // --Step3 : bind()
    if(bind(server_socket, (struct sockaddr *)&server_address,  sizeof(server_address))<0){ 
        perror("bind failed");
        exit(1);
    }
    logObj.log("(As server) Binding completed!");

    // Make it listen to incoming connections
    if(listen(server_socket, 3) < 0){ 
        perror("listen");
        exit(1);
    }
    logObj.log("(As server) Client is listening...");

    // Accept incoming connections and push them in thread vector
    vector<thread> thread_vector;
    while(true){
        int client_socket;
        if((client_socket = accept(server_socket, (struct sockaddr *)&server_address, (socklen_t *)&addr_len)) < 0){
            perror("Incoming connecton error");
            logObj.log("Error in Incoming connecton");
        }
        logObj.log("Connection Accepted");
        thread_vector.push_back(thread(driveClientReq, client_socket));
    }
    for(auto it=thread_vector.begin();it!=thread_vector.end();it++)
        if(it->joinable()) it->join();
    close(server_socket);
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
    logObj.log("Current Client : " + curr_client->addr);

    // Set log file for current tracker
    logObj = Logger("../logs", TYPE, argv[2]);

    // Create socket for client (treat it like a server)
    int server_socket = 0;
    struct sockaddr_in server_address;
    pthread_t server_thread;
    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket");
        return -1;
    }
    if(pthread_create(&server_thread, NULL, peerAsServer, NULL) == -1){
        perror("pthread");
        return -1;
    }

    // Connect to tracker
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(stoi(curr_tracker->curr_port));
    if(inet_pton(AF_INET, curr_tracker->curr_ip.c_str(), &server_address.sin_addr)<=0){
        logObj.log("Tracker address not valid!");
        return -1;
    }
    if(connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0){
        logObj.log("Error while connecting to tracker!");
        return -1;
    }
    logObj.log("Connected to tracker : "+curr_tracker->curr_addr);

    /* Busy waiting on commands */
    while(true){ 
        /*************************Process command from self*******************************/
        sleep(1);
        cout<<"Enter Command : ";
        string input_s, s;
        getline(cin, input_s);
        stringstream ss(input_s);
        vector <string> curr_input;
        while(ss >> s)
            curr_input.push_back(s);
        
        /*************************Run Commands*******************************/
        // Corner Condition 1 -> User trying to login but is already logged in
        if(curr_input[0] == "login" && curr_client->is_logged_in){
            logObj.log("Session is already active in this client instance!");
            continue;
        }

        // Corner Condition 2 -> User trying other commands but is not logged in (except create_user)
        if(curr_input[0] != "login" && curr_input[0] != "create_user" && !curr_client->is_logged_in){
            logObj.log("Login or create an account first!");
            continue;
        }

        // Send the command to the server
        send(server_socket , &input_s[0] , strlen(&input_s[0]) , MSG_NOSIGNAL);
        commandHandler(curr_input, server_socket);
        continue;
    }

    // Exit Client
    logObj.log("Quiting Client...");
    close(server_socket);
    return 0;
}