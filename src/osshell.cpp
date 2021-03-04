#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <filesystem>
#include <sys/wait.h>

namespace fs = std::filesystem;



void splitString(std::string text, char d, std::vector<std::string>& result);
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result);
void freeArrayOfCharArrays(char **array, size_t array_length);
bool fileExist(std::string path);


int main (int argc, char **argv)
{
   
    std::vector<std::string> os_path_list;
    char* os_path = getenv("PATH");
    splitString(os_path, ':', os_path_list);
    std::string inputCommand;
    std::vector<std::string> command_list; // to store command user types in, split into its variour parameters
    char **command_list_exec; // command_list converted to an array of character arrays
    std::vector<std::string> commands_history;




    // Welcome message
    std::cout << "Welcome to OSShell! Please enter your commands ('exit' to quit)." << std::endl;


    //file read code
    std::string line;
    int i = 0;
    std::ifstream historyFile; 
    historyFile.open("history.txt");
    if (historyFile.is_open()) {
        while (getline(historyFile,line)) {
            std::string tempLine= line;
            commands_history.push_back(tempLine);
            i++;
        }
        historyFile.close();
    }
    //end file read code

    while (true) {
        std::cout << "osshell> ";
        std::getline(std::cin,inputCommand);
        splitString(inputCommand, ' ', command_list);
        vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
        if (inputCommand.empty()) {
            

        }else if(inputCommand[0] == '.' || inputCommand[0] == '/'){
            //check if user input starts with a dot or slash
            if(fileExist(inputCommand) == true){
                
                char* pathAsChar = new char[inputCommand.size()];
                strcpy(pathAsChar, inputCommand.c_str());
                splitString(inputCommand, ' ', command_list);
                vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
                
                pid_t pid = fork();
                if(pid == 0){
                    if(execv(pathAsChar, command_list_exec) < 0){
                        std::cout << command_list[0] << ":" << " Error command not found" << std::endl;
                    }
                }
                int sd;
                waitpid(pid, &sd, 0);




            }else{
                std::cout << command_list[0] << ":" << " Error command not found" << std::endl;
            }

            commands_history.push_back(inputCommand);//Add the new command to the list of commands

        }else if (inputCommand == "exit") {
            //file write code
            std::ofstream historyFile ("history.txt");
            if (historyFile.is_open()) {
                for(int i = 0; i < commands_history.size(); i++) {
                    historyFile << commands_history[i] + "\n";
                }
                historyFile.close();
            }
            //end file write code
            break;
        }else if (inputCommand == "history"){
            
            //For loop to print out all of the commands saved
            for(int i = 0; i < commands_history.size(); i++) {
                    std:: cout << "  " << i+1 << ": " << commands_history[i].c_str() << std::endl;
                }

            commands_history.push_back(inputCommand);//Add the new command to the list of commands
        }
        else if (command_list[0] == "history") {
            if (command_list[1] == "clear") {
                for(int i = 0; i < commands_history.size(); i++) {
                    commands_history.clear();
                }
                
            } else if (atoi(command_list[1].c_str()) > 0 && atoi(command_list[1].c_str()) < commands_history.size()){
                
                int numberOfEntries = commands_history.size();
                int numberEntered = atoi(command_list[1].c_str());
                int newNUMB = numberOfEntries - numberEntered;
                for(int i = newNUMB; i < numberOfEntries; i++) {
                    std:: cout << "  " << i << ": " << commands_history[i] << std::endl;
                }   

                commands_history.push_back(inputCommand);//Add the new command to the list of commands             
            }else {
                std::cout << command_list[0] << ":" << " Error command not found" << std::endl;
                commands_history.push_back(inputCommand);//Add the new command to the list of commands   
            }

            command_list[1] = "";    
        }
        else {
            /*
            The user input is not 'exit', 'history', or start with dot or slash
            */

            commands_history.push_back(inputCommand);//Add the new command to the list of commands
            bool fileFound = false;
            std::string temp_path;
            std::string slash = "/";
            for(int i = 0; i < os_path_list.size() && fileFound == false; i++) {
                temp_path = os_path_list[i] + slash + command_list[0];
                fileFound = fileExist(temp_path);
            }

            char* pathAsChar = new char[inputCommand.size()];
            strcpy(pathAsChar, temp_path.c_str());
            if (fileFound == false) {
                std::cout << command_list[0] << ":" << " Error command not found" << std::endl;
            } else {
                pid_t pid = fork();
                if(pid == 0){
                    execv(pathAsChar, command_list_exec);
                }
                int sd;
                waitpid(pid, &sd, 0);
            }

        }
        
        
    }
    // Repeat:
    //  Print prompt for user input: "osshell> " (no newline)
    //  Get user input for next command
    //  If command is `exit` exit loop / quit program
    //  If command is `history` print previous N commands
    //  For all other commands, check if an executable by that name is in one of the PATH directories
    //   If yes, execute it
    //   If no, print error statement: "<command_name>: Error command not found" (do include newline)
    return 0;
}

bool fileExist(std::string path) {
    if (std::filesystem::exists(path) ){
        return true;
    } else {
        return false;
    }
}

/*
   text: string to split
   d: character delimiter to split `text` on
   result: vector of strings - result will be stored here
*/
void splitString(std::string text, char d, std::vector<std::string>& result)
{
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++)
    {
        char c = text[i];
        switch (state) {
            case NONE:
                if (c != d)
                {
                    if (c == '\"')
                    {
                        state = IN_STRING;
                        token = "";
                    }
                    else
                    {
                        state = IN_WORD;
                        token = c;
                    }
                }
                break;
            case IN_WORD:
                if (c == d)
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
            case IN_STRING:
                if (c == '\"')
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
        }
    }
    if (state != NONE)
    {
        result.push_back(token);
    }
}


/*
   list: vector of strings to convert to an array of character arrays
   result: pointer to an array of character arrays when the vector of strings is copied to
*/
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result)
{
    int i;
    int result_length = list.size() + 1;
    *result = new char*[result_length];
    for (i = 0; i < list.size(); i++)
    {
        (*result)[i] = new char[list[i].length() + 1];
        strcpy((*result)[i], list[i].c_str());
    }
    (*result)[list.size()] = NULL;
}

/*
   array: list of strings (array of character arrays) to be freed
   array_length: number of strings in the list to free
*/
void freeArrayOfCharArrays(char **array, size_t array_length)
{
    int i;
    for (i = 0; i < array_length; i++)
    {
        if (array[i] != NULL)
        {
            delete[] array[i];
        }
    }
    delete[] array;
}