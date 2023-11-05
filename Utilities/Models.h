#include <iostream>
using namespace std;

#define ll long long int

class Tracker {
    public:
        string ip;
        string port;
        string curr_ip;
        string curr_port;
        string curr_addr;
        unordered_map <string, string> user_creds;
        unordered_map <string, bool> is_user_logged_in;
        unordered_map<string, string> clientIdPort;
        vector<string> group_list;
        unordered_map<string, string> group_admins;
        unordered_map<string, set<string>> group_members;
        unordered_map<string, set<string>> group_requests;
        unordered_map<string, unordered_map<string, set<string>>> seeder_list;
        unordered_map<string, string> hash_per_piece;
        unordered_map<string, string> file_size;

        Tracker(string ip, string port, string curr_ip, string curr_port){
            this->ip = ip;
            this->port = port;
            this->curr_ip = curr_ip;
            this->curr_port = curr_port;
            this->curr_addr = curr_ip + ":" + curr_port;
        }

}*curr_tracker;

class Client {
    public:
        string ip;
        string port;
        string addr;
        bool is_logged_in;
        unordered_map <string, unordered_map <string, bool>> is_uploaded;
        unordered_map <string, vector <int>> file_piece_info;
        vector<vector <string>> file_pieces_peers;
        unordered_map <string, string> file_to_filepath;
        vector <string> piece_wise_hash;
        unordered_map <string, string> downloadedFiles;

        Client(string ip, string port, string addr){
            this->ip = ip;
            this->port = port;
            this->addr = ip + ":" + port;
        }

}*curr_client;

class FileDetails {
    public:
        string peer_ip;
        string file_name;
        ll file_size;

        FileDetails(string peer_ip, string file_name, ll file_size){
            this->peer_ip = peer_ip;
            this->file_name = file_name;
            this->file_size = file_size;
        }
};

class PieceDetails {
    public:
        ll piece_no;
        string peer_ip;
        string file_name;
        string dest;

        PieceDetails(ll piece_no, string peer_ip, string file_name, string dest){
            this->piece_no = piece_no;
            this->peer_ip = peer_ip;
            this->file_name = file_name;
            this->dest = dest;
        }
};

extern Tracker *curr_tracker;
extern Client *curr_client;
