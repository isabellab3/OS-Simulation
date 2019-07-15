#include <string>
#include <sstream>
#include <iostream>

#include "PCB.h"
#include "disk.h"
#include "OS.h"

using namespace std;

int main() {

    unsigned int RAM = 0;
    unsigned int page_size = 0;
    int number_of_hard_disks = 0;

    std::cout << "How much RAM is there?" << std::endl;
    std::cin >> RAM;

    std::cout << "What is the size of a page/frame?" << std::endl;
    std::cin >> page_size;

    std::cout << "How many hard disks does the simulated computer have?" << std::endl;
    std::cin >> number_of_hard_disks;


    OperatingSystem OS(number_of_hard_disks, RAM, page_size);

    std::string input;
    std::getline(std::cin, input);

    while (1) {
         //Shows which process is currently using the CPU and which processes are waiting in the ready-queue.
        if (input == "S r") {
            OS.Snapshot();
        }
        //Shows which processes are currently using the hard disks and which processes are waiting to use them
        else if (input == "S i") {
            OS.IOSnapshot();
        }
        //Shows the state of memory.
        else if (input == "S m") {
            OS.MemorySnapshot();
        }
        // Creates a new pcb and places it at end of ready queue, or in the CPU if the ready queue is empty.
        else if (input == "A") {
            OS.CreateProcess();
        }
        //The currently running process has spent a time quantum using the CPU. 
        else if (input == "Q") {
            OS.CPUToReadyQueue();
        }
        //The process using the CPU forks a child.The child is placed in the end of the ready - queue.
        else if (input == "fork") {
            OS.Fork();
        }
        //The process that is currently using the CPU terminates.
        else if (input == "exit") {
            OS.Exit();
        }
        //The process wants to pause and wait for any of its child processes to terminate.
        else if (input == "wait") {
            OS.Wait();
        }
        
        // For other commands, parse input
        std::stringstream in_stream(input);
        string first_word;
        in_stream >> first_word;

        if (first_word == "d" || first_word == "D" || first_word == "m") {
            int second_word;
            in_stream >> second_word;
            //The process that currently uses the CPU requests the hard disk #number. 
            //It wants to read or write file file _name.
            if (first_word == "d") {
                string third_word;
                in_stream >> third_word;
                OS.RequestDisk(second_word, third_word);
            }
            else if (first_word == "D") { // "D number") {
                // The hard disk #number has finished the work for one process.
                OS.RemoveProcessFromDisk(second_word);
            }
            //The process that is currently using the CPU requests a memory operation for the logical address.
            else if (first_word == "m") { // == "m address") {
                OS.RequestMemoryOperation(second_word);
            }
        }       
        // Get next line of input from user
        std::cout << endl;
        std::getline(std::cin, input);
    }
}