#ifndef HEADER 
#define HEADER

#include <bits/stdc++.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <iostream>
#include <time.h>
#include <ctime>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

using namespace std;

#define ll long long int
#define PIECE_SIZE 524288  // 512KB - Piece Size
#define LINE_SIZE 65536  // 64KB - Size

// commands
#define CREATE_USER "create_user"
#define LOGIN "login"
#define LOGOUT "logout"
#define CREATEGROUP "create_group"
#define LISTGROUP "list_groups"
#define JOINGROUP "join_group"
#define LEAVEGROUP "leave_group"
#define LISTREQ "list_requests"
#define ACCEPTREQ "accept_request"
#define LISTFILES "list_files"
#define UPLOADFILE "upload_file"
#define DOWNLOADFILE "download_file"
#define STOPSHARE "stop_share"

// messages
#define GROUP_NOT_FOUND "Group not found! Please try again"
#define NOT_ADMIN_OF_GROUP "You're not authorized as you're not admin of this group!"
#define USER_NOT_IN_GROUP "Current user not this group! Please try again"
#define NOT_MEMBER_GROUP "You're a not a member of this group! Please try again"
#define FILE_NOT_EXISTS "File path doesn't exists! Please try again"
#define FOLDER_NOT_EXISTS "Folder path doesn't exists! Please try again"
#define UPLOAD_SUCCESS "Upload is successful"
#define ARGS_COUNT_INCORRECT "Arguments count is not valid! Please check"
#define NO_PENDING_REQS "No pending requests!"
#define LOGIN_SUCCESS "Login is successful"
#define INVALID_CREDS "Incorrect Credentials. Please try again"
#define ALREADY_LOGGED_IN "User is already logged in. Session is active"
#define FILE_NOT_SHARED "Requested File is not shared in this group! Please try again"
#define SHARING_STOPPED "Stopped sharing of the requested file"
#define FILE_ALREADY_UP "File already uploaded!"

#endif