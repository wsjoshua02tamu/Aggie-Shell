#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <string.h>
#include <ctime>
#include "Tokenizer.h"

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;



int main () {
    string currDir = "";
    string prevDir = "";
    int old_stdin = dup(0);
    int old_stdout = dup(1);
    for (;;) {
        //implement iteration over vector of bg pid (vector also declared outside loop)

        char* env = getenv("USER");
        time_t currTime = time(NULL);
        // need date/time, username, and absolute path to current dir
        cout << YELLOW << "Shell$" << NC << " " << ctime(&currTime) << env << get_current_dir_name() << " ";
        
        // get user inputted command
        string input;
        getline(cin, input);


        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        vector<string> input_vec;
        string w="";
        for(auto i:input){
            if(i==';'){
            input_vec.push_back(w);
            w="";
        }
        else{
           w+=i;
        }
        }
        input_vec.push_back(w);

        for (auto str : input_vec){
        
        

        


        // get tokenized commands from user input
        //Tokenizer tknr(input);
        Tokenizer tknr(str);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }



        // // print out every command token-by-token on individual lines
        // // prints to cerr to avoid influencing autograder
        for (auto cmd : tknr.commands) {
             for (auto str : cmd->args) {
                 cerr << "|" << str << "| ";
             }
             if (cmd->hasInput()) {
                 cerr << "in< " << cmd->in_file << " ";
             }
             if (cmd->hasOutput()) {
                 cerr << "out> " << cmd->out_file << " ";
             }
            cerr << endl;
        }

        //chdir
        if (tknr.commands.at(0)->args.at(0) == "cd")
        {
            if (tknr.commands.at(0)->args.size() == 1)
            {
                struct passwd *pw = getpwuid(getuid());
                currDir = pw->pw_dir;
            }
            else
            {
                currDir = tknr.commands.at(0)->args.at(1);
            }

            if (currDir == "-")
            {
                currDir = prevDir;
            }
            else if (currDir == "~"){
                struct passwd *pw = getpwuid(getuid());
                currDir = pw->pw_dir;
            }
            prevDir = get_current_dir_name();
            chdir(currDir.c_str());
            continue;
        }
        

        //if dir (cd <dir>) is the -, go to prev directory
        //variable storing prev directory (declared outside loop)

        //for piping
        size_t counter = 0;
        for (auto cmd: tknr.commands){
        
        
        //  call pipe() to make pipe
        int fds[2];
        pipe(fds);
        //  fork() in child
        //add checks for first/last command
        


        // fork to create child
        pid_t pid = fork();
        if (pid < 0) {  // error check
            perror("fork");
            exit(2);
        }

        

        //char* args[1000];
        //add check for bg process - add pid to vector if bg and don't waitpid() in parent

        if (pid == 0) {  // if child, exec to run 
            char* args[1000];
            // run single commands with no arguments
            // implement multiple arguments; iterate over args of current command to make char* 
            
            for (size_t j = 0; j < cmd->args.size(); j++){
                    args[j] = (char*) cmd->args.at(j).c_str();
            }
            args[cmd->args.size()] = nullptr;
            
            //char* args[] = {(char*) tknr.commands.at(0)->args.at(0).c_str(), nullptr};
            
            if (cmd->hasInput()){
                int inputFile = open(cmd->in_file.c_str(), O_RDONLY);
                dup2(inputFile, 0);
                //dup2(0,inputFile);
            }
            if (cmd->hasOutput()){
                int outputFile = open(cmd->out_file.c_str(), O_RDWR | O_CREAT, 0666);
                dup2(outputFile, 1);
            }
            //if current command is redirected, then open file and dup2 std(in/out) that's being redirected
            
            if (counter < tknr.commands.size()-1){
                dup2(fds[1],1);
            }

            close(fds[0]);
            
            //implement it safely for both at same time


            if (execvp(args[0], args) < 0) {  // error check
                perror("execvp");
                exit(2);
            }
        }
        else {  // if parent, wait for child to finish
            dup2(fds[0],0);
            close(fds[1]);
            int status = 0;
            if (!cmd->isBackground()){
            waitpid(pid, &status, 0);
            
            if (status > 1) {  // exit if child didn't exec properly
                exit(status);
            }
            }
        }
    counter++;
    }
    dup2(old_stdin, 0);
    dup2(old_stdout, 1);
    }
    }
    
}
