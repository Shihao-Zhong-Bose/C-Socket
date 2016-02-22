
#include "Structure.cpp"

using namespace std;

int main()
{
    client* entity = new client(9001,"127.0.0.1",0);
    connect();
    entity->startListen(3);

}