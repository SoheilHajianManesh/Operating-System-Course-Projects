#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>

#include "loger.h"
#include "defs.h"
#include "connection.h"
#include "besides.h"
#include "json.h"
#include "ansi-color.h"

Restaurant rst;

int alarmstat = 0;
void alarmhandler2(int sig)
{
    alarmstat = 1;
}

void showRequests()
{
    printf("username/port/order\n");
    for (int i = 0; i < rst.requests.size; i++)
    {
        printf("%s %d %s\n", rst.requests.arr[i].name, rst.requests.arr[i].tcpSocket, rst.requests.arr[i].foodName);
    }
}

void showSales()
{
    printf("username/order/result\n");
    for (int i = 0; i < rst.sales.size; i++)
    {
        char *result;
        if (rst.sales.arr[i].status == ACC)
        {
            result = "accept";
            printf("%s%s%s %s%s%s %s%s%s\n", ANSI_YEL, rst.sales.arr[i].name,
                   ANSI_RST, ANSI_MAG, rst.sales.arr[i].foodName, ANSI_RST, ANSI_GRN, result, ANSI_RST);
        }
        else if (rst.sales.arr[i].status == DENIED)
        {
            result = "reject";
            printf("%s%s%s %s%s%s %s%s%s\n", ANSI_YEL, rst.sales.arr[i].name,
                   ANSI_RST, ANSI_MAG, rst.sales.arr[i].foodName, ANSI_RST, ANSI_RED, result, ANSI_RST);
        }
        else if (rst.sales.arr[i].status == TIMEOUT)
        {
            result = "TIMEOUT";
            printf("%s%s%s %s%s%s %s%s%s\n", ANSI_YEL, rst.sales.arr[i].name,
                   ANSI_RST, ANSI_MAG, rst.sales.arr[i].foodName, ANSI_RST, ANSI_BLU, result, ANSI_RST);
        }
    }
}

void moveRequestToSales(int index)
{
    if (index < 0 || index >= rst.requests.size)
    {
        logError("Invalid request index");
        return;
    }
    Request requestToMove = rst.requests.arr[index];
    for (int i = index; i < rst.requests.size - 1; i++)
    {
        rst.requests.arr[i] = rst.requests.arr[i + 1];
    }
    rst.requests.size--;

    if (rst.sales.size == 0)
    {
        rst.sales.arr = (Request *)malloc(sizeof(Request));
    }
    else
    {
        rst.sales.arr = (Request *)realloc(rst.sales.arr, (rst.sales.size + 1) * sizeof(Request));
    }

    rst.sales.arr[rst.sales.size] = requestToMove;
    rst.sales.size++;
    logMsg(rst.username, "Request moved to sales successfully.");
}

void addRequestToRestaurant(const char *foodName, const char *name, int tcpPort)
{
    if (rst.requests.arr == NULL)
    {
        rst.requests.arr = (Request *)malloc(MAX_REQUESTS * sizeof(Request));
        if (rst.requests.arr == NULL)
        {
            logError("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        rst.requests.size = 0;
    }

    if (rst.requests.size < MAX_REQUESTS)
    {
        Request newRequest;
        strcpy(newRequest.foodName, foodName);
        strcpy(newRequest.name, name);
        newRequest.status = WAITED;
        newRequest.tcpSocket = tcpPort;

        rst.requests.arr[rst.requests.size] = newRequest;
        rst.requests.size++;
        logMsg(rst.username, "Request added to the restaurant successfully.");
    }
    else
    {
        logError("Request array is full, cannot add more requests.");
    }
}

int findRequestIndexByTcpPort(int tcpPort)
{
    for (int i = 0; i < rst.requests.size; i++)
    {
        if (rst.requests.arr[i].tcpSocket == tcpPort)
        {
            return i;
        }
    }
    return -1;
}

int findFoodIndexByName(char *name)
{
    for (int i = 0; i < rst.foods.numOfFoods; i++)
    {
        if (!strcmp(rst.foods.food[i].name, name))
        {
            return i;
        }
    }
    return -1;
}

int findIngredientIndexByName(char *name)
{
    for (int i = 0; i < rst.ingredients.arrSize; i++)
    {
        if (!strcmp(rst.ingredients.arr[i].name, name))
        {
            return i;
        }
    }
    return -1;
}

int haveEnoughIngredient(int index)
{
    char *name = rst.requests.arr[index].foodName;
    int foodIdx = findFoodIndexByName(name);
    if (foodIdx == -1)
    {
        logError("Food does not exist.");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < rst.foods.food[foodIdx].ings.numOfIngs; i++)
    {
        int needed = rst.foods.food[foodIdx].ings.arr[i].amount;
        int ingIdx = findIngredientIndexByName(rst.foods.food[foodIdx].ings.arr[i].name);
        if (needed > rst.ingredients.arr[ingIdx].amount)
        {
            return FALSE;
        }
    }
    for (int i = 0; i < rst.foods.food[foodIdx].ings.numOfIngs; i++)
    {
        int needed = rst.foods.food[foodIdx].ings.arr[i].amount;
        int ingIdx = findIngredientIndexByName(rst.foods.food[foodIdx].ings.arr[i].name);
        rst.ingredients.arr[ingIdx].amount -= needed;
    }
    logMsg(rst.username, "List of ingredients updated successfully.");
    return TRUE;
}

void answerRequest(FdSet *fdSet)
{
    char reqPort[MAX_PORT_LEN];
    char answer[MAX_MSG];
    char msg[MAX_MSG];
    sprintf(msg, "-> %sport%s of request: ", ANSI_MAG, ANSI_RST);
    getInput(STDOUT_FILENO, msg, reqPort, MAX_PORT_LEN);
    int requestPort = atoi(reqPort);
    sprintf(msg, "-> your answer (%syes%s/%sno%s): ", ANSI_GRN, ANSI_RST, ANSI_RED, ANSI_RST);
    getInput(STDOUT_FILENO, msg, answer, MAX_MSG);
    int index = findRequestIndexByTcpPort(requestPort);
    if (index == -1)
    {
        logError("Port does not exist.");
        return;
    }
    if (!strcmp(answer, "yes"))
    {
        if (haveEnoughIngredient(index) == FALSE)
        {
            strcpy(answer, "no");
            logError("Do not have enough ingredient.");
        }
        else
        {
            rst.requests.arr[index].status = ACC;
            logMsg(rst.username, "Order accepted successfully.");
        }
    }
    if (!strcmp(answer, "no"))
    {
        rst.requests.arr[index].status = DENIED;
        logMsg(rst.username, "Order rejected successfully.");
    }
    sprintf(msg, "%s %s", answer, rst.username);
    write(requestPort, msg, MAX_MSG);
    FD_CLEARER(rst.requests.arr[index].tcpSocket, fdSet);
    close(rst.requests.arr[index].tcpSocket);
    moveRequestToSales(index);
    logMsg(rst.username, "Close tcp port successfully.");
}

void getOrderInfo(int OrderSocket, FdSet *fdSet)
{
    char info[MAX_MSG];
    char msg[MAX_MSG];
    int result = recv(OrderSocket, info, MAX_MSG, 0);
    if (result == 0)
    {
        int index = findRequestIndexByTcpPort(OrderSocket);
        rst.requests.arr[index].status = TIMEOUT;
        moveRequestToSales(index);
        FD_CLEARER(OrderSocket, fdSet);
        logWarning("Request timeout");
        logMsg(rst.username, "Time to answer to order end.");
        return;
    }
    if (rst.status == CLOSE)
    {
        logMsg(rst.username, "A request was rejected because restaurant close.");
        sprintf(msg, "close");
        write(OrderSocket, msg, MAX_MSG);
        close(OrderSocket);
        FD_CLEARER(OrderSocket, fdSet);
        return;
    }
    char *foodName = tokenizeStr(info, DELIMETER);
    char *name = tokenizeStr(info, DELIMETER);
    addRequestToRestaurant(foodName, name, OrderSocket);
    sprintf(msg, "%snew%s order!\n", ANSI_GRN, ANSI_RST);
    write(STDOUT_FILENO, msg, strlen(msg));
    logMsg(rst.username, "Get a new order successfully.");
}

void addIngredient(const char *name, int amount)
{
    for (int i = 0; i < rst.ingredients.numOfIngs; i++)
    {
        if (!strcmp(rst.ingredients.arr[i].name, name))
        {
            rst.ingredients.arr[i].amount += amount;
            return;
        }
    }
    logError("Ingredient does not exist.");
}

void showSuppliers()
{
    char msg[BCAST_MSG];
    char response[BCAST_MSG] = {'\0'};
    signal(SIGALRM, alarmhandler);
    siginterrupt(SIGALRM, 1);
    sprintf(msg, "username/port\n");
    write(STDOUT_FILENO, msg, strlen(msg));
    sprintf(msg, "suppliers %d", rst.tcpPort);
    sendBcast(rst.bcast.addr, rst.bcast.bcastSocket, msg);
    logMsg(rst.username, "Sent Wanting restaurants on brodcast successfully.");
    while (1)
    {
        int accSocket = -1;
        alarm(1);
        accSocket = acceptClient(rst.serverSocket);
        alarm(0);
        if (accSocket < 0)
        {
            break;
        }
        memset(response, 0, BCAST_MSG);
        recv(accSocket, response, BCAST_MSG, 0);
        write(STDOUT_FILENO, response, strlen(response));
        write(STDOUT_FILENO, "\n", 1);
    }
}

void brodCastAction(char *brodcBuff)
{
    char infoMsg[BCAST_MSG];
    char *brodcPart = tokenizeStr(brodcBuff, DELIMETER);
    if (!strcmp(brodcPart, "restaurants"))
    {
        int port = strToPort(tokenizeStr(brodcBuff, DELIMETER));
        sprintf(infoMsg, "%s %s %s %d %s", ANSI_YEL, rst.username, ANSI_MAG, rst.tcpPort, ANSI_RST);
        sendWantedInformation(port, infoMsg);
        logMsg(rst.username, "Send name and port to someone requested successfully.");
    }
    else if (!strcmp(brodcPart, "username"))
    {
        brodcPart = tokenizeStr(brodcBuff, DELIMETER);
        if (!strcmp(brodcPart, rst.username))
        {
            sprintf(infoMsg, DUP_ERROR);
            sendBcast(rst.bcast.addr, rst.bcast.bcastSocket, infoMsg);
            logMsg(rst.username, "Send duplicate warning on broadcast successfully.");
        }
    }
    else if (!strcmp(brodcPart, "display"))
    {
        logMsg(rst.username, "Display brodcast message successfully.");
    }
    else
    {
        logMsg(rst.username, "Recive something on broadcast that is not important.");
    }
}

void requestIngredient()
{
    char msg[MAX_MSG];
    char response[MAX_MSG];
    char supplierPort[MAX_PORT_LEN];
    char nameOfIngredient[NAME_SIZE];
    char numberOfIngredient[MAX_NUMBER_LEN];
    sprintf(msg, "-> %sPort%s of Supplier: ", ANSI_MAG, ANSI_RST);
    getInput(STDIN_FILENO, msg, supplierPort, MAX_PORT_LEN);
    int suppPort = strToPort(supplierPort);
    sprintf(msg, "-> name of %singredient%s: ", ANSI_YEL, ANSI_RST);
    getInput(STDIN_FILENO, msg, nameOfIngredient, NAME_SIZE);
    sprintf(msg, "-> number of %singredient%s: ", ANSI_YEL, ANSI_RST);
    getInput(STDIN_FILENO, msg, numberOfIngredient, MAX_NUMBER_LEN);
    int numOfIng = atoi(numberOfIngredient);
    int connectSocket;
    int result = connectServer(suppPort, &connectSocket);
    if (result < 0)
    {
        logError("connection failed");
        return;
    }
    logMsg(rst.username, "connect to supplier successfully.");
    signal(SIGALRM, alarmhandler2);
    siginterrupt(SIGALRM, 1);
    sprintf(msg, "request %d", rst.tcpPort);
    write(connectSocket, msg, strlen(msg));
    sprintf(msg, "%swaiting%s for supplier's response ... \n", ANSI_YEL, ANSI_RST);
    write(STDOUT_FILENO, msg, strlen(msg));
    alarm(90);
    int res = recv(connectSocket, response, MAX_MSG, 0);
    alarm(0);
    char *token = tokenizeStr(response, DELIMETER);
    if (alarmstat == 1 || res == 0)
    {
        logError("Request Timeout.");
        logMsg(rst.username, "Request for ingredient timeout.");
    }
    else if (!strcmp(token, "full"))
    {
        logError("Supplier is busy.");
        logMsg(rst.username, "Supplier is busy.");
    }
    else if (!strcmp(token, "yes"))
    {
        addIngredient(nameOfIngredient, numOfIng);
        token = tokenizeStr(response, DELIMETER);
        sprintf(msg, "%s%s%s supplier %saccepted%s\n", ANSI_YEL, token, ANSI_RST, ANSI_GRN, ANSI_RST);
        write(STDOUT_FILENO, msg, strlen(msg));
        logMsg(rst.username, "Supplier accept your request.");
    }
    else if (!strcmp(token, "no"))
    {
        token = tokenizeStr(response, DELIMETER);
        sprintf(msg, "%s%s%s supplier %srejected%s\n", ANSI_YEL, token, ANSI_RST, ANSI_RED, ANSI_RST);
        write(STDOUT_FILENO, msg, strlen(msg));
        logMsg(rst.username, "Supplier reject your request.");
    }
    close(connectSocket);
    alarmstat = 0;
    logMsg(rst.username, "Connection with supplier closed successfully.");
}
void commandHandler(FdSet *fdSet)
{
    char cmdBuf[MAX_BUFF] = {'\0'};
    char msg[MAX_MSG] = {'\0'};
    getInput(STDIN_FILENO, NULL, cmdBuf, MAX_BUFF);
    char *cmdPart1 = tokenizeStr(cmdBuf, DELIMETER);
    char *cmdPart2 = tokenizeStr(cmdBuf, DELIMETER);
    if (cmdPart1 == NULL)
    {
        logError("Unknown command.");
    }
    if (!strcmp(cmdPart1, "start"))
    {
        if (rst.status == OPEN)
        {
            logError("Already open.");
        }
        else
        {
            rst.status = OPEN;
            sprintf(msg, "display %s%s%s restaurant %sopened%s!\n", ANSI_YEL, rst.username, ANSI_RST, ANSI_GRN, ANSI_RST);
            sendBcast(rst.bcast.addr, rst.bcast.bcastSocket, msg);
            logMsg(rst.username, "Send opening msg on broadcast successfully.");
        }
    }
    else if (!strcmp(cmdPart1, "break"))
    {
        if (rst.status == CLOSE)
        {
            logError("Already close.");
        }
        else
        {
            rst.status = CLOSE;
            sprintf(msg, "display %s%s%s restaurant %sclosed%s!\n", ANSI_YEL, rst.username, ANSI_RST, ANSI_RED, ANSI_RST);
            sendBcast(rst.bcast.addr, rst.bcast.bcastSocket, msg);
            logMsg(rst.username, "Send opening msg on broadcast successfully.");
        }
    }
    else if (!strcmp(cmdPart1, "show"))
    {
        if (!strcmp(cmdPart2, "ingredients"))
        {
            showIngredients(&rst.ingredients);
            logMsg(rst.username, "Show ingredients successfully.");
        }
        else if (!strcmp(cmdPart2, "recipes"))
        {
            showRecipes(&rst.foods);
            logMsg(rst.username, "Show recipes successfully.");
        }
        else if (!strcmp(cmdPart2, "suppliers"))
        {
            showSuppliers();
            logMsg(rst.username, "Show suppliers successfully.");
        }
        else if (!strcmp(cmdPart2, "request"))
        {
            showRequests();
            logMsg(rst.username, "Show waiting requests successfully.");
        }
        else if (!strcmp(cmdPart2, "sales"))
        {
            showSales();
            logMsg(rst.username, "Show sales successfully.");
        }
        else
            logError("Unknown command.");
    }
    else if (!strcmp(cmdPart1, "request") && !strcmp(cmdPart2, "ingredient"))
    {
        requestIngredient();
        logMsg(rst.username, "Requesting to a supplier successfully.");
    }
    else if (!strcmp(cmdPart1, "answer") && !strcmp(cmdPart2, "request"))
    {
        answerRequest(fdSet);
        logMsg(rst.username, "Answering to a customer successfully.");
    }
    else
        logError("Unknown command.");
}
void interface()
{
    char msgBuf[MAX_BUFF];
    char brodcBuf[MAX_BUFF] = {0};
    FdSet fdSet;
    fdSet.maxFd = 0;
    FD_ZERO(&fdSet.masterSet);
    FD_SETTER(STDIN_FILENO, &fdSet);
    FD_SETTER(rst.bcast.bcastSocket, &fdSet);
    FD_SETTER(rst.serverSocket, &fdSet);
    while (1)
    {
        cliPrimary();
        memset(msgBuf, '\0', MAX_BUFF);
        fdSet.workingSet = fdSet.masterSet;
        select(fdSet.maxFd + 1, &fdSet.workingSet, NULL, NULL, NULL);
        for (int i = 0; i <= fdSet.maxFd; ++i)
        {
            if (!FD_ISSET(i, &fdSet.workingSet))
                continue;
            if (i != STDIN_FILENO)
            {
                write(STDOUT_FILENO, "\x1B[2K\r", 5);
            }
            if (i == STDIN_FILENO)
            {
                commandHandler(&fdSet);
            }
            else if (i == rst.bcast.bcastSocket)
            {
                memset(brodcBuf, 0, 1024);
                recv(i, brodcBuf, 1024, 0);
                logMsg(rst.username, "Recive something on broadcast.");
                brodCastAction(brodcBuf);
                logMsg(rst.username, "Action on brodcast successfully.");
            }
            else if (i == rst.serverSocket)
            {
                int acceptSock = acceptClient(rst.serverSocket);
                FD_SETTER(acceptSock, &fdSet);
                logMsg(rst.username, "New restaurant accept as client successfully.");
            }
            else
            {
                getOrderInfo(i, &fdSet);
            }
        }
    }
}

void initBrodcast(const char *port)
{
    rst.udpPort = strToPort(port);
    rst.bcast.bcastSocket = makeBroadcast(BRODCAST_IP, rst.udpPort, &rst.bcast.addr);
    logInfo("Broadcast receiving successfully initialized.");
}

void initServer()
{
    while (1)
    {
        rst.tcpPort = generateRandomPort();
        rst.serverSocket = makeServer(rst.tcpPort);
        if (rst.serverSocket >= 0)
            break;
    }
    logInfo("Making server succesfully.");
}

void checkDuplicate()
{
    signal(SIGALRM, alarmhandler);
    siginterrupt(SIGALRM, 1);
    char responseBuf[BCAST_MSG];
    char brodMsg[BCAST_MSG];
    sprintf(brodMsg, "username %s", rst.username);
    int r = sendBcast(rst.bcast.addr, rst.bcast.bcastSocket, brodMsg);

    while (1)
    {
        int res = -1;
        memset(responseBuf, 0, BCAST_MSG);
        alarm(1);
        res = recv(rst.bcast.bcastSocket, responseBuf, BCAST_MSG, 0);
        alarm(0);
        if (res < 0)
            return;
        if (!strcmp(responseBuf, DUP_ERROR))
        {
            logError("Duplicate username!");
            getInput(STDIN_FILENO, "Enter your username: ", rst.username, NAME_SIZE);
            sprintf(brodMsg, "username %s", rst.username);
            sendBcast(rst.bcast.addr, rst.bcast.bcastSocket, brodMsg);
        }
    }
}

void initUsername()
{
    getInput(STDIN_FILENO, "Please enter your username: ", rst.username, NAME_SIZE);
    checkDuplicate();
}

void initRestaurant(const char *port)
{
    rst.status = CLOSE;
    getFoods(&rst.foods);
    extractIngredients(&rst.foods, &rst.ingredients);
    initBrodcast(port);
    initServer();
    initUsername();
    createLogFile(rst.username);
    logMsg(rst.username, "Restaurant created successfully.");
    char welcomeMsg[MAX_MSG];
    sprintf(welcomeMsg, "welcome %s %s %s as restaurant\n", ANSI_GRN, rst.username, ANSI_RST);
    write(STDOUT_FILENO, welcomeMsg, strlen(welcomeMsg));
}
int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        logError("Wrong number of arguments.");
        return EXIT_FAILURE;
    }
    initRestaurant(argv[1]);
    interface();
}