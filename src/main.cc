#include <iostream>
#include <cell.h>

using namespace socket_project;

int main(int argc, char const* argv[]){
    Cell *cell_server = new Cell();
    Cell *cell_client1 = new Cell();
    Cell *cell_client2 = new Cell();
    Cell *cell_client3 = new Cell();

    cell_server->server(8080);
    cell_client1->join(8080);
    cell_client2->join(8080);
    cell_client3->join(8080);

    int x;
    while(true){
        std::cin >> x;
    }
    return 0;
}