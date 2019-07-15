#ifndef DISK_H
#define DISK_H

#include <list>
#include <string>

class HardDisk {
    public:
        HardDisk() : current_process(-1), current_file(""), io_queue(0) {}
        
        ~HardDisk() {}

        // A process with the given pid requests to use the disk to read/write the file file_name.
        void Request(const std::string & file_name, const int & pid) {
            // If there is no process using the disk already, it can go straight to the disk
            if (DiskIsIdle()) {
                current_process = pid;
                current_file = file_name;
            }
            // Otherwise add the process to the io queue
            else {
                io_queue.push_back(std::pair<int, std::string>(pid, file_name));
            }
        }

        // The process currently using the disk is removed and returns to the ready queue. If there is another process waiting
        // to use the disk, it can come from the io queue.
        int RemoveProcess() {
            int removed_process = current_process;
            if (!DiskIsIdle()) {
                // No process is waiting to use the disk; set idle
                if (io_queue.empty()) {
                    current_process = -1;
                    current_file = "";
                }

                // Otherwise let the next process on the queue use the disk
                else {
                    current_process = io_queue.front().first;
                    current_file = io_queue.front().second;
                    io_queue.pop_front();
                }
            }
            return removed_process;
        }

        // Removes the given process if it is found using the disk or in its io queue
        void Remove(const int & pid) {
            // If the process is using the disk
            if (current_process == pid) {
                RemoveProcess();
            }
            // If the process is in the io queue
            for (auto itr = io_queue.begin(); itr != io_queue.end(); itr++) {
                if ((*itr).first == pid) {
                    io_queue.erase(itr);
                }
            }
        }

        // Returns the pid of the process currently using the disk
        int GetCurrentProcess() const {
            return current_process;
        }

        // Returns the name of the file that the current process is reading or writing
        std::string GetCurrentFile() const {
            return current_file;
        }

        // Sets the current process to the given pid
        void SetCurrentProcess(const int & pid) {
            current_process = pid;
        }

        // Returns true if the disk is not being used by any process. Otherwise returns false
        bool DiskIsIdle() const {
            return current_process == -1;
        }

        // Prints the process using the disk, the file it is reading/writing, and the items on the io queue.
        void PrintQueue() const {
            for (auto itr = io_queue.begin(); itr != io_queue.end(); itr++) {
                std::cout << "<- [" << (*itr).first << " " << (*itr).second << "] ";
            }
            std::cout << std::endl;
        }

    private:
        int current_process;                                    // Pid of the process currently using the hard disk. Set to -1 when idle
        std::string current_file;                               // The name of the file the current process is reading/writing
        std::list<std::pair<int, std::string>> io_queue;        // The pids of processes waiting to use the CPU, and the names of the files associated with them
};

#endif // DISK_H
