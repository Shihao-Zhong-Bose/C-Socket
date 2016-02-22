#include "Structure.cpp"

using namespace std;

int main()
{
    server* entity = new server(9001,"128.101.37.2",0);

    entity->startListen(10);


}