#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <pthread.h>

#define NUMTHRDS 2
#define PORTA 20032
#define ERRO -1
#define TAMMAX 250 // tamanho maximo da string
pthread_mutex_t server_done;

pthread_t t[NUMTHRDS];

static void *client(void *_)
{
    pthread_mutex_lock(&server_done);
    struct sockaddr_in network;

    int sock,
        newSock,
        resp,
        strucsize;

    char msg[TAMMAX];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == ERRO)
    {
        perror("Socket");
        exit(0);
    }

    bzero((char *)&network, sizeof(network));
    network.sin_family = AF_INET;
    network.sin_port = htons(PORTA);
    network.sin_addr.s_addr = inet_addr("127.0.0.1");

    strucsize = sizeof(network);

    resp = connect(sock, (struct sockaddr *)&network, strucsize);

    if (resp == ERRO)
    {
        perror("Connect");
        exit(0);
    }

    fprintf(stdout, "Cliente conectado em %s\n", "127.0.0.1");

    for (;;)
    {
        pthread_mutex_lock(&server_done);
        printf("\nMensagem: ");
        fgets(msg, TAMMAX, stdin);
        send(sock, msg, sizeof(msg), 0);
    }
    exit(0);
}

static void *server(void *_)
{
    struct sockaddr_in network,
        local;
    int sock,
        newSock,
        resp,
        strucsize;

    char msgbuffer[TAMMAX];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == ERRO)
    {
        perror("Socket");
        exit(0);
    }
    bzero((char *)&local, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(PORTA);
    local.sin_addr.s_addr = INADDR_ANY;

    strucsize = sizeof(local);

    resp = bind(sock, (struct sockaddr *)&local, strucsize);

    if (resp == ERRO)
    {
        perror("Bind");
        exit(0);
    }

    fprintf(stdout, "Servidor conectado em %s\n", "127.0.0.1");

    pthread_mutex_unlock(&server_done);

    listen(sock, 5);

    newSock = accept(sock, (struct sockaddr *)&network, &strucsize);

    if (newSock == ERRO)
    {
        perror("Accept");
        exit(0);
    }

    fprintf(stdout, "\nRecebendo conexao de: %s\n", inet_ntoa(network.sin_addr));
    pthread_mutex_unlock(&server_done);

    for (;;)
    {
        pthread_mutex_unlock(&server_done);
        recv(newSock, msgbuffer, TAMMAX, 0);
        fprintf(stdout, "\nMensagem Recebida: %s\n", msgbuffer);
        if (strcmp(msgbuffer, "exit") == 0)
        {
            exit(0);
        }
    }
}

int main()
{
    printf("\nIniciando programa...\n");

    pthread_mutex_init(&server_done, NULL);
    pthread_mutex_lock(&server_done);
    pthread_create(&t[0], NULL, server, NULL);
    pthread_create(&t[1], NULL, client, NULL);

    pthread_mutex_destroy(&server_done);
    pthread_exit(NULL);
    return 1;
}