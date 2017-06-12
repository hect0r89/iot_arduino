//
// Created by hector on 12/06/17.
//
/*
    C socket server example, handles multiple clients using threads
*/

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread

//the thread function
void *connection_handler(void *);
void *start_timer(void *);
void *sockets_thread(void *);

int start = 0;
int game_over = 0;
int *TIME_TO_START = (int *) 15;

int main(int argc, char *argv[]) {
    int *socket_desc;
    struct sockaddr_in server;

    //Create socket
    socket_desc = (int *) socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == (int *) -1) {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    //Bind
    if (bind((int) socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Creamos el hilo que maneja los jugadores
    pthread_t socket_thread;
    if (pthread_create(&socket_thread, NULL, sockets_thread, socket_desc) < 0) {
        perror("could not create thread");
        return 1;
    }

    //El juego se ejecuta en bucle
    while (1) {
        game_over = 0;
        start = 0;
        //Accept and incoming connection
        puts("Esperando jugadores...");

        // Hilo que ejecuta la cuenta atrás para empezar a jugar
        pthread_t sniffer_thread;
        if (pthread_create(&sniffer_thread, NULL, start_timer, TIME_TO_START) < 0) {
            perror("could not create thread");
            return 1;
        }

        // Cuando el juego acaba se reinicia la partida
        while (!game_over) {

        }
        puts("Reiniciando...");
        sleep(10);

    }
    return 0;
}


void *sockets_thread(void *socket_desc) {
    int c, client_sock, *new_sock;
    struct sockaddr_in client;
    c = sizeof(struct sockaddr_in);
    pthread_t sniffer_thread;

    //Listen
    listen((int) socket_desc, 3);

    while ((client_sock = accept((int) socket_desc, (struct sockaddr *) &client, (socklen_t *) &c))) {
        puts("Connection accepted");
        new_sock = malloc(1);

        //Se crea un hilo por cada nuevo jugador
        *new_sock = client_sock;
        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *) new_sock) < 0) {
            perror("could not create thread");
        }
        puts("Jugador conectado");
    }


    if (client_sock < 0) {
        perror("accept failed");
    }

    return 0;
}

/**
 * Método que inicia una cuenta atrás para que el juego comience
 * @param time tiempo de espera
 * @return
 */
void *start_timer(void *time) {
    int t = (int) time;
    do {
        printf("T: %d\n", t);
        t--;
        sleep(1);
    } while (t > 0);
    // Cuando el tiempo acaba se cambia la variable Start para que los jugadores esperando puedan jugar
    start = 1;
    sleep(2);
    start = 0;
    return 0;
}

/**
 * Método que gestiona a cada jugador
 * @param socket_desc socket del jugador
 * @return
 */
void *connection_handler(void *socket_desc) {
    //Get the socket descriptor
    int sock = *(int *) socket_desc;
    int read_size, number_of_lifes = 3;
    char client_message[2000];

    if ((read_size = (int) recv(sock, client_message, 2000, 0)) > 0) {
        puts(client_message);
        //write(sock , number_of_lifes+"\n" , strlen(number_of_lifes));
        if (read_size == 0) {
            puts("Client disconnected");
            fflush(stdout);
        } else if (read_size == -1) {
            perror("recv failed");
        };
    }

    //El jugador se mantiene a la espera hasta que la cuenta atrás le de paso
    while (!start) {

    };

    write(sock, "Start\n", strlen("Start\n"));
    puts("Start");

    while ((read_size = (int) recv(sock, client_message, 2000, 0)) > 0) {
        if (!game_over) {
            number_of_lifes = atoi(client_message);
            printf("Vidas: %s", client_message);
            if (atoi(client_message) == 0) {
                game_over = 1;
                break;
            }
        } else {
            break;
        }
        if (read_size == 0) {
            puts("Client disconnected");
            fflush(stdout);
        } else if (read_size == -1) {
            perror("recv failed");
        };
    }
    puts("END");
    write(sock, "End\n", strlen("End\n"));

    if (number_of_lifes > 0) {
        write(sock, "W\n", strlen("W\n"));
    } else {
        write(sock, "L\n", strlen("L\n"));
    }
    shutdown(sock, 2);
    return 0;
}
