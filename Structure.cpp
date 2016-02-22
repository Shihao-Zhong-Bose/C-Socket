#include "Structure.h"
#include <stdio.h>
#include <string.h>
#include <bitset>
#include <netinet/tcp.h>
struct inputMethod
{
	int i;
	sockaddr_in* address;
	int sockFd;
};

struct ParsedRequest
{
    std::string* arr;
    int len;
};

client::client(int port , std::string ip , int protocol)
{	

	this->port = port;
	this->ip = ip;
	this->protocol = protocol;
	sockAddress.sin_port = htons(port);
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = inet_addr(ip.c_str());

	if (protocol == 0)
	{
		sockFd = socket(PF_INET , SOCK_STREAM , 0);
	}

	else
	{
		sockFd = socket(PF_INET , SOCK_DGRAM , 0);
	}

	if (sockFd < 0)
	{
		throw std::runtime_error("socket created failure");
	}

	if(bind(sockFd,(struct sockaddr *)&sockAddress,
		sizeof(sockAddress)) < 0)
	{
		perror("Error on binding");
	}
}


server::server(int port , std::string ip , int protocol)
{

	this->port = port;
	this->ip = ip;
	this->protocol = protocol;
	sockAddress.sin_port = htons(port);
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_addr.s_addr = inet_addr(ip.c_str());

	if (protocol == 0)
	{
		sockFd = socket(PF_INET , SOCK_STREAM , 0);
	}

	else
	{
		sockFd = socket(PF_INET , SOCK_DGRAM , 0);
	}

	if (sockFd < 0)
	{
		throw std::runtime_error("socket created failure");
	}
	
	int optval = 1;
	//setsockopt(sockFd,IPPROTO_TCP,TCP_NODELAY,&optval, sizeof(char));
	setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval));

	if(bind(sockFd,(struct sockaddr *)&sockAddress,
		sizeof(sockAddress)) < 0)
	{
		perror("Error on binding");
	}
} 


std::string trim(std::string &str)   
{  
   if (str.empty())   
   {  
      return str;  
   }   
   str.erase(0, str.find_first_not_of(" "));  
   str.erase(str.find_last_not_of(" ") + 1);

   return str;
}

ParsedRequest * strToArr(std::string str)
{
	using namespace std;
    int len=0;
    std::string temp = str;
    while(str.length()>0)
   {
        str= trim(str);

        int split = str.find(" ");
        std::string cut = str.substr(0,split);
        str = str.substr(split+1,str.length());

        len++;

        if(split==-1)
        {
            break;
        }
   }


   //initialize array
   std::string* strarr = new string[len];

   int i=0;
   while(temp.length()>0){

        temp= trim(temp);

        int split = temp.find(" ");
        std::string cut = temp.substr(0,split);
        temp = temp.substr(split+1,temp.length());

        strarr[i] = cut; 

        i++;

        if(split==-1)
        {
            break;
        }
   }


   ParsedRequest* result = new ParsedRequest();
   result->arr = strarr;
   result->len = len;

   return result;
}

int getFileSize(std::string fileName){
	std::fstream fd(fileName.c_str(),std::ios::out | std::ios::in | std::fstream::binary);
	fd.seekg(0,std::ios_base::end);
	return fd.tellg();
}

char* readFile(std::string fileName)
{
	using namespace std;
	std::fstream fd(fileName.c_str(),std::ios::out | std::ios::in | std::fstream::binary);
	int size = getFileSize(fileName);
	char* data = new char[size];
	
	while(!fd.eof())
	{
		fd.read(data,size);
	}
	
	//cout <<"data = " <<bitset<8>(data)<<endl;

	fd.close();
	return data;

}

int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 onm failure, 0 on success
} 


void* threadAccept(void* input)
{
	using namespace std;
	inputMethod* method = (inputMethod*)input;
	sockaddr_in* addr = method->address;
	int i = method->i;
	int sockfd = method->sockFd;

	//receive request message
	char* buffer = new char[8192];
	ssize_t temp = recv(sockfd,buffer,8192,0);
	//parse http request 
	ParsedRequest * result = strToArr(string(buffer));
	//cout<< result->arr[0]<< " " <<result->arr[1] << " "<<result->arr[2]<<endl;



	// send message
	const char * header= "HTTP/1.0 200 OK \n\n";
	ifstream test(result->arr[1].substr(1).c_str());
	if (!test)
	{
		delete buffer;
		cout<< "no such file"<< endl;
		close(sockfd);
		pthread_exit(NULL);
	}
	char* data = readFile(result->arr[1].substr(1));
	int size = getFileSize(result->arr[1].substr(1));
	//printf("data = %c", data );

	send(sockfd,header,strlen(header),MSG_NOSIGNAL);
	int image = 0;
	sendall(sockfd,data,&size);
	
		//image  = send(sockfd,data,size,MSG_NOSIGNAL) ;


	cout << "image sent : "<<image<<endl;
	close(sockfd);
	
	delete buffer;
	pthread_exit(NULL);

}





void server::startListen(int listenTime)
{
	pthread_t thread[listenTime];


	if(listen(sockFd,listenTime) < 0)
	{
		throw std::runtime_error("listen failure");
	}

	struct sockaddr_in* address[listenTime];
	struct inputMethod* method[listenTime];


	int addrlen = sizeof(struct sockaddr_in);

	for (int i = 0 ; i < listenTime ; i++)
	{
		address[i] = new sockaddr_in();
		method[i] = new inputMethod();
	}

	while(true)
	{
		for (int i = 0 ; i < listenTime ; i++)
		{
			int acceptSock = accept (sockFd , (struct sockaddr *)address[i] , (socklen_t*)&addrlen);
			method[i]->sockFd = acceptSock;
			method[i]->i = i;
			method[i]->address = address[i];
			std::cout << "start"<<std::endl;
			if (pthread_create(&thread[i],NULL,threadAccept,(void*)method[i]))
			{
				throw std::runtime_error("thread created failure. Connection number overflow");
			}
		}
	}
}
