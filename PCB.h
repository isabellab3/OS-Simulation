#ifndef PCB_H
#define PCB_H

#include <vector>


class PCB {
    public:

        PCB(int & pid_) : pid(pid_), child_processes(0), parent_process(0), process_is_zombie(0), waiting(0) {}
        
        ~PCB() {
            for (auto child : child_processes) {
                delete child;
            }
            ClearChildren();
        }
        
        int GetPid() const {
            return pid;
        }

        // Adds a pointer to a child process of this pcb
        void AddChildProcess(PCB* child_process) {
            child_processes.push_back(child_process);
        }

        // Sets the process to waiting or not waiting
        void SetWaitingState(bool state) {
            waiting = state;
        }
        
        // Marks the process as a zombie (1) or not a zombie (0)
        void SetZombie(bool state) {
            process_is_zombie = state;
        }

        // Returns true if the process is waiting, otherwise returns false
        bool IsWaiting() {
            return waiting;
        }

        // Returns true if the process is a zombie process. Returns false otherwise
        bool IsZombieProcess() const {
            return process_is_zombie;
        }

        // If the process has a child that is a zombie, returns the pid of that child. If it does not, returns 0.
        int ProcessHasZombieChild() const {
            for (auto itr = child_processes.begin(); itr != child_processes.end(); itr++) {
                if ((*itr)->process_is_zombie) {
                    return (*itr)->pid;
                }
            }
            return 0;
        }

        //Returns true if the process has children.
        bool HasChildren() const {
            return !child_processes.empty();
        }
        
        // When called on a process, removes the given child if it is found in child_processes.
        void RemoveChild(PCB* & child) {
            for (auto itr = child_processes.begin(); itr != child_processes.end(); itr++) {
                if ((*itr) == child) {
                    child_processes.erase(itr);
                    return;
                }
            }
        }

        // Sets the parent pid of a process
        void SetParent(const int parent_pid) {
            parent_process = parent_pid;
        }

        // Returns the parent pid of a process
        int GetParent() {
            return parent_process;
        }

        // Returns a constant reference to the vector of child processes.
        const std::vector<PCB*> & GetChildren() const {
            return child_processes;
        }

        // Clears the vector of child processes.
        void ClearChildren() {
            child_processes.clear();
        }
        
        
    private:
        int pid;                                // Unique id of the process
        std::vector<PCB*> child_processes;      // Pointers of all children of the process
        int parent_process;                     // The pid of the parent of the process
        bool process_is_zombie;                 // True is process is a zombie process, false otherwise
        bool waiting;                           // True if this process is waiting for a child process to terminate.
};

#endif // PCB_H
