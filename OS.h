#ifndef OS_H
#define OS_H

#include "PCB.h"
#include "disk.h"

#include <algorithm>
#include <vector>
#include <list>
#include <map>

class OperatingSystem {
    public:
        OperatingSystem(int & number_of_hard_disks_, unsigned int & RAM_, unsigned int & page_size_) : 
            CPU(1), 
            number_of_processes(1), 
            timestamp(0), 
            number_of_hard_disks(number_of_hard_disks_), 
            page_size(page_size_), 
            RAM(RAM_),
            number_of_frames(RAM_ / page_size_), 
            ready_queue(0), 
            hard_disks(number_of_hard_disks_), 
            frames(0) {
                
            // Creates initial process
            PCB* process_1 = new PCB{ number_of_processes };
            process_1->SetParent(1); //First process has no parent
            all_processes[number_of_processes] = process_1;

            // Initialize hard disks
            for (int i = 0; i < number_of_hard_disks; i++) {
                HardDisk* disk_ = new HardDisk{};
                hard_disks[i] = disk_;
            }
        }

        ~OperatingSystem() {
            // Delete disks
            for (auto disk : hard_disks) {
                delete disk;
            }
            // Delete frames
            for (auto frame : frames) {
                delete frame;
            }
            // Delete all PCBs
            for (auto PCB : all_processes) {
                delete PCB.second;
            }

            hard_disks.clear();
            frames.clear();
            all_processes.clear();
        }

        // Creates a new process and adds it to the ready queue, or the CPU if it is empty.
        // The parent of the new process is process 1.
        void CreateProcess() {
            PCB* new_process = new PCB{ ++number_of_processes };
            all_processes[number_of_processes] = new_process;
            
            // Parent process is 1
            all_processes[1]->AddChildProcess(new_process);
            new_process->SetParent(1);

            // Adds new process to ready queue
            AddToReadyQueue(number_of_processes);
        }

        
        // Creates a new process whose parent is the pcb currently using CPU.
        void Fork() {
            if (CPU == 1) { // If fork called with no process in CPU
                std::cout << "There is no process in the CPU to fork" << std::endl;
            }
            else {
                PCB* new_process = new PCB(++number_of_processes);

                // Parent process is the process in the CPU that called fork
                all_processes[CPU]->AddChildProcess(new_process); 
                new_process->SetParent(CPU);

                // Add new process to ready queue
                AddToReadyQueue(number_of_processes);

                all_processes[number_of_processes] = new_process;
            }
        }

        // Adds the given process to the ready queue, or if the CPU is idle, the process goes straight there instead.
        void AddToReadyQueue(const int pid) {
            // First process goes straight to CPU
            if (CPU == 1) {
                CPU = pid;
            }
            // Otherwise process added to back of queue
            else {
                ready_queue.push_back(pid);
            }
        }

        // Shows the process currently using the CPU, and lists any processes in the ready queue.
        void Snapshot() const {
            std::cout << "Process using CPU: " << CPU << std::endl;
            std::cout << "Ready Queue:  ";
            for (auto itr = ready_queue.begin(); itr != ready_queue.end(); ++itr) {
                std::cout <<  " <- " << *itr;
            }
            std::cout << std::endl;
        }


        // Moves the process currently running in CPU to the end of the ready queue
        // Also places a new process into the CPU, if there is one in the ready queue
        void CPUToReadyQueue() {    
            ready_queue.push_back(CPU);
            CPU = ready_queue.front();
            ready_queue.pop_front();
        }

        // The process using the CPU calls wait.
        void Wait() {
            PCB* cpu_process = all_processes[CPU];

            // If the process has no children, nothing to wait for
            if (cpu_process->HasChildren()) {
                // If the process already has at least one zombie child, child disappears and parent keeps using CPU
                int PID_of_zombie_child = cpu_process->ProcessHasZombieChild();
                if (PID_of_zombie_child > 0) {  ///////////???
                    cpu_process->SetWaitingState(0);

                    // Delete the zombie child's children
                    DeleteChildren(all_processes[PID_of_zombie_child]);
                    
                    // Delete the zombie child
                    PCB* zombie = all_processes[PID_of_zombie_child];
                    PCB* parent = all_processes[zombie->GetParent()];

                    parent->RemoveChild(zombie);
                    all_processes.erase(PID_of_zombie_child);
                    delete zombie;
                    zombie = nullptr;
                }

                // If the process has no zombie children, remove the process from the ready queue and send process at front of ready queue to CPU
                else {
                    // Set process using CPU to waiting
                    cpu_process->SetWaitingState(1);
                    CPU = ready_queue.front();
                    ready_queue.pop_front();
                    // The waiting parent process will be added back to the end of the ready queue when one of its children exits.
                }
            }
        }

		//The process that is currently using the CPU terminates. If its parent is already waiting, the process terminates immediately and the parent 
		// goes to the end of the ready queue. If the parent isn't waiting, the process becomes a zombie process. If the parent is process 1, the 
		// process terminates immediately. All children of the process are terminated.
        void Exit() {
            PCB* exiting_process = all_processes[CPU];
            PCB* parent = all_processes[exiting_process->GetParent()];

            //If parent is waiting, the process (and all of its children) terminate immediately, and the parent goes to the end of the ready queue.
            if (parent->IsWaiting()) {
                // Terminate all children and the process
                DeleteChildren(exiting_process);
                all_processes.erase(exiting_process->GetPid());
                parent->RemoveChild(exiting_process);
                delete exiting_process;
                exiting_process = nullptr;

                // Parent goes to the end of the ready queue and is no longer waiting, and the process exits the CPU
                parent->SetWaitingState(0);
                AddToReadyQueue(parent->GetPid());
                GetNextFromReadyQueue();
            }
            //If parent is process 1, terminate the process and all children immediately.
            else if (parent->GetPid() == 1) {
                //Terminate children
                DeleteChildren(exiting_process);

                //Terminate process
                all_processes.erase(exiting_process->GetPid());
                parent->RemoveChild(exiting_process);
                delete exiting_process;
                exiting_process = nullptr;

                // Replace the process in the CPU
                GetNextFromReadyQueue();

            }
            // Parent is not waiting, so process becomes a zombie.
            else {
                // Mark process as a zombie
                exiting_process->SetZombie(1);

                // Terminate all children
                DeleteChildren(exiting_process);
                GetNextFromReadyQueue();
            }
        }

        // Removes a process from the ready queue, if the ready queue contains the process
        void RemoveFromReadyQueue(const int pid_) {
            auto pid_to_delete = std::find(ready_queue.begin(), ready_queue.end(), pid_);
            // If the item is in the ready queue, removes it
            if (pid_to_delete != ready_queue.end()) {
                ready_queue.erase(pid_to_delete);
            }
        }

        //Checks each hard disk for the given process. If the process is using any of the disks or is on one of their io queues, it is removed.
        void RemoveFromDisks(const int & pid_) {
            for (auto itr = hard_disks.begin(); itr != hard_disks.end(); itr++) {
                (*itr)->Remove(pid_);
            }
        }


        //Shows the state of memory.
        //For each used frame, displays the process number that occupies it and the page number stored in it.
        //The enumeration of pages and frames starts from 0.
        void MemorySnapshot() {
            std::cout << "Frame   " << "Page Number     " << "pid       " << "ts" << std::endl;
            for (unsigned int i = 0; i < frames.size(); i++) {
                std::cout << "  " <<  i << "        ";
                if (frames[i]->pid_ != 0) {
                    std::cout << "   " << frames[i]->page_ << "          " << frames[i]->pid_ << "         " << frames[i]->timestamp_;
                }
                std::cout << std::endl;
            }
        }

        //The process that is currently using the CPU requests a memory operation for the logical address.
        void RequestMemoryOperation(const int & address) {
            int page = address / page_size;
            
            int oldest_timestamp = timestamp;
            int index_of_oldest = -1;

            // Adds frames to the vector as needed until the vector is full (size is number_of_frames). Then replaces the least recently used frame
            // If the same process wants to access the same page, just update time stamp
            for (unsigned int i = 0; i < frames.size(); i++) {
                if ((frames[i]->page_ == page) && (frames[i]->pid_ == CPU)) {
                    frames[i]->timestamp_ = timestamp;
                    timestamp++;
                    return;
                }
            }

            // If there are empty frames, create a new frame in the vector
            if (frames.size() < number_of_frames) {
                Frame* new_frame = new Frame{};
                new_frame->timestamp_ = timestamp;
                new_frame->pid_ = CPU;
                new_frame->page_ = page;
                frames.push_back(new_frame);
            }

            // Replace the least recently used frame's data
            else {
                // Find the frame with the oldest(lowest) timestamp
                for (unsigned int i = 0; i < frames.size(); i++) {
                    if (frames[i]->timestamp_ <= oldest_timestamp) {
                        index_of_oldest = i;
                        oldest_timestamp = frames[i]->timestamp_;
                    }
                }
                frames[index_of_oldest]->page_ = page;
                frames[index_of_oldest]->pid_ = CPU;
                frames[index_of_oldest]->timestamp_ = timestamp;
            }
            timestamp++;
        }

        // Shows which processes are currently using the hard disks and what processes are waiting to use them.
        void IOSnapshot() const {
            for (int i = 0; i < number_of_hard_disks; i++) {
                std::cout << "Disk " << i << ": ";
                if (hard_disks[i]->DiskIsIdle()) {
                    std::cout << "idle" << std::endl;
                }
                else {
                    std::cout << "[" << hard_disks[i]->GetCurrentProcess() << " " << hard_disks[i]->GetCurrentFile() << "]" << std::endl;
                    std::cout << "Queue for disk " << i << ": ";
                    hard_disks[i]->PrintQueue();
                }
            }
        }

        // The process using the CPU requests the hard disk disk_number
        // It wants to read or write file file _name.
        void RequestDisk(const int & disk_number, const std::string & file_name) {
            if ((disk_number < number_of_hard_disks) && (disk_number >= 0)) {
                // Process 1 should not use any disks/be added to any queues
                if (CPU != 1) {
                    hard_disks[disk_number]->Request(file_name, CPU);
                    // Remove from CPU and replace from ready queue
                    GetNextFromReadyQueue();
                }
            }
            else {
                std::cout << "There is no disk " << disk_number << std::endl;
            }
        }

        // The process at the front of the ready queue is removed and moves to the CPU.
        void GetNextFromReadyQueue() {
            if (ready_queue.empty()) {
                CPU = 1;
            }
            else {
                CPU = ready_queue.front();
                ready_queue.pop_front();
            }
        }

        // When a process is done using a disk, puts it back onto the ready queue.
        void RemoveProcessFromDisk(const int & disk_number) {
            if ((disk_number < number_of_hard_disks) && (disk_number >= 0)) {
                int removed_pcb = hard_disks[disk_number]->RemoveProcess();
                AddToReadyQueue(removed_pcb);
            }
            else {
                std::cout << "There is no disk " << disk_number << std::endl;
            }
        }

        //Checks each frame for the given process. If the process is found it is removed.
        void RemoveFromFrames(const int & pid) {
            for (auto itr = frames.begin(); itr != frames.end(); itr++) {
                if ((*itr)->pid_ == pid) {
                    (*itr)->Clear();
                }
            }
        }

    private:
        int CPU;   								// The pid of the process currently using the CPU
        int number_of_processes;    			// Not the current number of processes, but keeps track of how many are created while the program runs.
        int timestamp;							// For keeping track of memory requests
        const int number_of_hard_disks;      	
        const unsigned int page_size;
        const unsigned int RAM;
        const unsigned int number_of_frames;                  
        std::list<int> ready_queue;				// Holds the pids of processes waiting on the ready queue
        std::vector<HardDisk*> hard_disks; 		// Index of the vector is the disk number (disk 0 to disk n), holding a pointer to that disk
        std::map<int, PCB*> all_processes;   	// A map of all processes; The key is the pid of the process, the value is the pointer to that process
     
        struct Frame {
            int timestamp_;
            int page_;
            int pid_;

            Frame() : timestamp_(0), page_(0), pid_(0) {}
            ~Frame() {}

            bool IsEmpty() {
                return pid_ == 0;
            }
            
            void Clear() {
                timestamp_ = 0;
                page_ = 0;
                pid_ = 0;
            }
        };

        std::vector<Frame*> frames;

        // Deletes all children of a process, and removes them and the process pcb from all disks, frames, their queues and the ready queue.
        void DeleteChildren(PCB* pcb) {
            //Delete all children of the process pcb_to_delete
            for (auto itr = pcb->GetChildren().begin(); itr != pcb->GetChildren().end(); itr++) {
                PCB* child = (*itr);
                if (child->HasChildren()) {
                    DeleteChildren(child);
                }
                else {
                    RemoveFromDisks(child->GetPid());
                    RemoveFromFrames(child->GetPid());
                    RemoveFromReadyQueue(child->GetPid());

                    // Delete child from all_processes
                    all_processes.erase(child->GetPid());
                    delete child;
                    child = nullptr;
                }
            }

            pcb->ClearChildren();

            // Keep zombies around and delete later
            if (!pcb->IsZombieProcess()) {
                all_processes.erase(pcb->GetPid());
            }

            RemoveFromDisks(pcb->GetPid());
            RemoveFromFrames(pcb->GetPid());
            RemoveFromReadyQueue(pcb->GetPid());
        }
};


#endif //OS_H
