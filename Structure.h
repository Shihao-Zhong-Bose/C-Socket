#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <exception>
#include <stdexcept>
#include <pthread.h>
#include <fstream>

class client;
class server;


class dataTrans
{
public:

	template<typename T>
	ssize_t sends(int sockfd , T* buf)
	{
		ssize_t msg = send(sockfd,(void*)buf,sizeof(buf),MSG_CONFIRM);
		return msg;
	}

	template<typename T>
	ssize_t recvs(int sockfd , T* buf)
	{
		ssize_t msg = recv(sockfd,(void*)buf,sizeof(buf),MSG_WAITALL);
		return msg;
	}

};


class client: public dataTrans
{
public:
	struct sockaddr_in sockAddress;
	int sockFd;
	int port;
	std::string ip;
	int protocol;


	client(int port , std::string ip , int protocol);
	int connect();
};


class server: public dataTrans
{
private:
	struct sockaddr_in sockAddress;
	int sockFd;
	int port;
	std::string ip;
	int protocol;


public:
	server(int port , std::string ip , int protocol);
	void startListen(int listenTime);
	char* readFile (std::string fileName);
	//std::string& trim(std::string &str)   ;
	//static void* threadAccept(void* inputmethod);
};

