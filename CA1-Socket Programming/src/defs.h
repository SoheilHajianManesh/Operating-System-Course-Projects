#ifndef DEFS_H_INCLUDE
#define DEFS_H_INCLUDE

#include <netinet/in.h>
#include <sys/select.h>


#define NAME_SIZE  64
#define BCAST_MSG  128
#define MAX_BUFF  128
#define MAX_MSG  128
#define BRODCAST_IP  "255.255.255.255"
#define MAX_PORT 65535
#define MAX_PORT_LEN 10
#define MAX_NUMBER_LEN 5
#define MAX_REQUESTS 20
#define INFO_BUFF 512
#define DELIMETER " "
#define TRUE 1
#define FALSE 0
#define DUP_ERROR "Duplicate"

typedef enum {
    OPEN = 0,
    CLOSE = 1,
} RestaurantStatus;

typedef enum{
    YES=0,
    NO=1
} Status;

typedef enum {
    ACC = 0,
    DENIED = 1,
    TIMEOUT=2,
    WAITED=3
} RequestStatus;

typedef struct{
    int maxFd;
    fd_set masterSet;
    fd_set workingSet;
}FdSet;

typedef struct {
    int bcastSocket;
    struct sockaddr_in addr;
}BcastInfo;

typedef struct {
    char name[NAME_SIZE];
    char foodName[NAME_SIZE];
    int status;
    int tcpSocket;
}Request;

typedef struct{
    Request* arr;
    int size;
}RequestArray;

typedef struct{
    char name[NAME_SIZE];
    int amount;
}Ingredient;

typedef struct {
    Ingredient* arr;
    int numOfIngs;
    int arrSize;
}IngredientsArray;

typedef struct
{
    char name[NAME_SIZE];
    IngredientsArray ings;
}Food;

typedef struct{
    Food* food;
    int numOfFoods;
}Foods;

typedef struct{
    int serverSocket;
    char username[NAME_SIZE];
    int udpPort;
    int tcpPort;
    int status;
    IngredientsArray ingredients;
    Foods foods;
    RequestArray requests;
    RequestArray sales;
    BcastInfo bcast;
}Restaurant;

typedef struct{
    int serverSocket;
    int tcpPort;
    char username[NAME_SIZE];
    int udpPort;
    BcastInfo bcast;
}Customer;

typedef struct{
    int serverSocket;
    char username[NAME_SIZE];
    int udpPort;
    int tcpPort;
    int clientSocket;
    int haveRequest;
    char requestAnswer[4];
    BcastInfo bcast;
}Supplier;



#endif