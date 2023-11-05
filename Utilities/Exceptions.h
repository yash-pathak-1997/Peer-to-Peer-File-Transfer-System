#include <iostream>
using namespace std;

class ArgumentsException : public exception {
    
    public:
        string printMessage (string type) {
            return (type + " : Arguments entered are not correct! Please try again.\nExiting....");
        }
};

class InfoFileNotExistsException : public exception {

    public:
        string printMessage (string type) {
            return (type + " : Info file doesn't exists! Please try again.\nExiting....");
        }

};

class SocketCreationException : public exception {

    public:

        string place;

        SocketCreationException(string place){
            this->place = place;
        }

        string printMessage (string type) {
            cout<< (type + " : Error in socket creation at "+place+".\n");
            return place;
        }

};