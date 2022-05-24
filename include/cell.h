#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>


namespace socket_project{

enum cellMode{
    SERVER = 0,
    CLIENT = 1,
};

class Cell{
    private:
        cellMode mode;
        int sock;
        sockaddr_in addr;
        int add_size;
        std::thread *thread_loop = nullptr;
        std::thread *thread_connectivity = nullptr;
        std::thread *thread_read = nullptr;
        bool listening = false;
        void connection_loop();
        void connectivity_loop();
        void read_socket();
        std::vector<int> connections;
        std::mutex* safety;

    public:
        Cell();
        ~Cell();
        void server(int __port);
        void stop();
        void join(int __port);
        void message(char const *__msg, int __fd);
        void broadcast(char const *__msg);
};

}