#include "TcpListener.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <cstdio>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int TcpListener::init()
{
	// Create a socket
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket < 0) {
		fprintf(stderr,"[ERROR:] cannot open socket");
	}

	// Bind the ip address and port to a socket
	struct sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(m_port);
	inet_pton(AF_INET, m_ipAddress, &hint.sin_addr);

	if (bind(m_socket, (sockaddr*)&hint, sizeof(hint)) < 0) {
		fprintf(stderr, "[ERROR:] at binding");
		return -1;
	}
	// Tell Winsock the socket is for listening 
	if (listen(m_socket, SOMAXCONN) < 0) {
		fprintf(stderr, "[ERROR:] at listen");
		return -1;
	} 

	// Create the master file descriptor set and zero it
	for (int i = 0; i < (MAX_CLIENTS + 1); ++i) {
		m_master[i].fd = -1;
		m_master[i].events = 0;
	}

	// Add our first socket that we're interested in interacting with; the listening socket!
	// It's important that this socket is added for our server or else we won't 'hear' incoming
	// connections 
	available = MAX_CLIENTS;
	m_master[0].fd = m_socket;

	return 0;
}

int TcpListener::run() {
	// this will be changed by the \quit command (see below, bonus not in video!)
	bool running = true;

	while (running) {
		std::cout << "[DEBUG:] Im HERE " << available << " out of " << MAX_CLIENTS << std::endl;
		// Make a copy of the master file descriptor set, this is SUPER important because
		// the call to select() is _DESTRUCTIVE_. The copy only contains the sockets that
		// are accepting inbound connection requests OR messages. 

		// E.g. You have a server and it's master file descriptor set contains 5 items;
		// the listening socket and four clients. When you pass this set into select(), 
		// only the sockets that are interacting with the server are returned. Let's say
		// only one client is sending a message at that time. The contents of 'copy' will
		// be one socket. You will have LOST all the other sockets.

		// SO MAKE A COPY OF THE MASTER LIST TO PASS INTO select() !!!

		// See who's talking to us
		// int socketCount = select(0, &copy, nullptr, nullptr, nullptr);
		m_master[0].events = (available > 0) ? POLLIN : 0;
		int socketCount = poll(m_master, MAX_CLIENTS, -1);
		std::cout << "socketCount " << socketCount << std::endl;
		// Is it an inbound communication?
		if (m_master[0].revents == POLLIN) {
			// Accept a new connection
			int client = accept(m_socket, nullptr, nullptr);

			// Add the new connection to the list of connected clients
			allocateClient(client);

			onClientConnected(client);
			socketCount--;
			std::cout << "I was asked" << std::endl;
		}	

		// Loop through all the current connections / potential connect
		// It's an inbound message
		int i = 1;
		while (socketCount > 0) {

			if(m_master[i].revents == POLLIN) {
				std::cout << "Client wants something" << std::endl;
				char buf[4096];
				memset(buf, 0, 4096);
				int sock = m_master[i].fd;

				// Receive message
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0) {
					// Drop the client
					onClientDisconnected(sock);
					close(sock);
					deallocateClient(sock);
					std::cout << "Leaving client" << std::endl;
				}
				else {
					onMessageReceived(sock, buf, bytesIn);
				}
			}
			i++;
			socketCount--;
		}
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	close(m_master[0].fd);

	int i = 1;
	while (available != MAX_CLIENTS) {

		if (m_master[i].fd) {
			// Get the socket number
			close(m_master[i].fd);
			// Remove it from the master file list and close the socket
			m_master[i].fd = -1;
			available++;
		}
		i++;
	}

	return 0;
}

void TcpListener::allocateClient(int client) {
	unsigned i = 1;

	while(m_master[i].fd > -1) i++;
	available--;

	m_master[i].fd = client;
	m_master[i].events = POLLIN;
}

void TcpListener::deallocateClient(int client) {
	unsigned i = 1;
	
	while(m_master[i].fd != client) i++;
	available++;
	
	m_master[i].fd = -1;
	m_master[i].events = 0;
}


void TcpListener::sendToClient(int clientSocket, const char* msg, int length) {
	send(clientSocket, msg, length, 0);
}

void TcpListener::broadcastToClients(int sendingClient, const char* msg, int length)
{
	for (int i = 1; i < available; i++) {
		int outSock = m_master[i].fd;
		if (outSock != sendingClient) {
			sendToClient(outSock, msg, length);
		}
	}
}

void TcpListener::onClientConnected(int clientSocket)
{

}

void TcpListener::onClientDisconnected(int clientSocket)
{

}

void TcpListener::onMessageReceived(int clientSocket, const char* msg, int length)
{

}
