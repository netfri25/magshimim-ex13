# Ex13 - Multi-Threaded Server

Link to ex13 google drive folder: <a href="https://drive.google.com/drive/u/0/folders/1adcHLL7cFOPtIJLuaaAt72P55iJwI6-p/" target="_blank">Ex13 Google Drive</a>

The last exercise in 'Advanced Programming' course first semester! <br />
This time we are going build a chat server, that manages clients and messages that clients send each other. <br />
Ex13 will require us to use OOP, Data Structures, Threads, Synchronization and Sockets. <br />
In this exercise, you will implement a server, and use a client app that was already built for you. <br />
The purpose of this exercise is to to build a system that uses most of what we learnt throughout the semester.

**Please note!**
To test your exercise, please use the tests found in this repo: <br />
[https://gitlab.com/tomeriq/magshimimex13tests](https://gitlab.com/tomeriq/magshimimex13tests)

### Demo
Contains files and information necessary to run a demo for this exercise. <br />
This demo is a good way to see what is the expected result, and how it should look after you are done.

### Helper.h/cpp
Files that contains static functions you can use in your exercise, make sure to use them, it will save you time!

### client_server_comm.pcap
A "packet capture" (.pcap) file, that contains an example of a simple client-server communication. <br />
Use [WireShark](https://www.wireshark.org/download.html) to open this file and see the message format and conversation flow you are expected to support.

### .gitlab-ci.yml
Pipeline file for auto-tests

## Submission details
Files to submit:

#### MagshiChat Server
Submit all the files required to run the server you implemented. <br />
Remember to include a *main.cpp* file.

#### MagshiChat Client
Remember to also submit the *MagshiChat.exe* file with the *config.txt* file you use to run the client side.

#### Bonus - Client Side Code
In this [link](https://drive.google.com/drive/u/0/folders/1tozvHHthI3TPbit7gnhS104yFQh1EGet) 
you can find the code files for client side application that connects to our server. <br />
You can use this code if you want to change or improve the client side code, this is a bonus!