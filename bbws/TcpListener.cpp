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
	// Initialze winsock
	/******************* for Windowss**********
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		return wsOk;
	}
	******************************************/
	// Create a socket
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket < 0) {
		fprintf(stderr,"[ERROR:] cannot open socket");
		//return WSAGetLastError();
	}

	// Bind the ip address and port to a socket
	struct sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(m_port);
	inet_pton(AF_INET, m_ipAddress, &hint.sin_addr);

	if (bind(m_socket, (sockaddr*)&hint, sizeof(hint)) < 0) {
		fprintf(stderr, "[ERROR:] at binding");
	}

	// Tell Winsock the socket is for listening 
	if (listen(m_socket, SOMAXCONN) < 0) {
		fprintf(stderr, "[ERROR:] at listen");
	}

	// Create the master file descriptor set and zero it
	// FD_ZERO(&m_master);
	for (int i = 0; i < (MAX_CLIENTS + 1); ++i) {
		m_master[i].fd = 0;
		m_master[i].events = POLLIN;
	}
	// Add our first socket that we're interested in interacting with; the listening socket!
	// It's important that this socket is added for our server or else we won't 'hear' incoming
	// connections 
	available = MAX_CLIENTS;
	m_master[0].fd = m_socket;
	// FD_SET(m_socket, &m_master);

	return 0;
}

int TcpListener::run() {
	// this will be changed by the \quit command (see below, bonus not in video!)
	bool running = true;

	while (running) {

		// Make a copy of the master file descriptor set, this is SUPER important because
		// the call to select() is _DESTRUCTIVE_. The copy only contains the sockets that
		// are accepting inbound connection requests OR messages. 

		// E.g. You have a server and it's master file descriptor set contains 5 items;
		// the listening socket and four clients. When you pass this set into select(), 
		// only the sockets that are interacting with the server are returned. Let's say
		// only one client is sending a message at that time. The contents of 'copy' will
		// be one socket. You will have LOST all the other sockets.

		// SO MAKE A COPY OF THE MASTER LIST TO PASS INTO select() !!!
		// fd_set copy = m_master;

		// See who's talking to us
		// int socketCount = select(0, &copy, nullptr, nullptr, nullptr);
		m_master[0].events = (available > 0) ? POLLIN : 0;
		int socketCount = poll(m_master, MAX_CLIENTS, -1);

		// Loop through all the current connections / potential connect
		// for (int i = 0; i < socketCount; i++)
		// {
		if (m_master[0].revents == POLLIN) {
			// Accept a new connection
			int client = accept(m_socket, nullptr, nullptr);

			// Add the new connection to the list of connected clients
			allocateClient(client);
			// FD_SET(client, &m_master);

			onClientConnected(client);
			socketCount--;
		}	

		int i = 1;
		while (socketCount > 0) {
			// Makes things easy for us doing this assignment
			// int sock = copy.fd_array[i];

			// Is it an inbound communication?
			// if (sock == m_socket) {
				// // Accept a new connection
				// int client = accept(m_socket, nullptr, nullptr);

				// // Add the new connection to the list of connected clients
				// FD_SET(client, &m_master);

				// onClientConnected(client);
			// }
			// else // It's an inbound message
			// {
			if(m_master[i].revents == POLLIN) {
				char buf[4096];
				memset(buf, 0, 4096);
				int sock = m_master[i].fd;

				// Receive message
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0) {
					// Drop the client
					onClientDisconnected(sock);
					close(sock);
					// FD_CLR(sock, &m_master);
					deallocateClient(sock);
				}
				else {
					onMessageReceived(sock, buf, bytesIn);
				}
			}
		}
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	// FD_CLR(m_socket, &m_master);
	// m_master[0].fd = 0;
	close(m_master[0].fd);

	// while (m_master.fd_count > 0)
	// {
	int i = 1;
	while (available != MAX_CLIENTS) {
		// Get the socket number
		//SOCKET sock = m_master.fd_array[0];

		// Remove it from the master file list and close the socket
		// FD_CLR(sock, &m_master);
		// close(sock);
		if (m_master[i].fd) {
			close(m_master[i].fd);
			m_master[i].fd = 0;
			available++;
		}
		i++;
	}

	// Cleanup winsock
	// WSACleanup();
	return 0;
}

void TcpListener::allocateClient(int client) {
	unsigned i = 1;

	while(m_master[i].fd) i++;
	available--;

	m_master[i].fd = client;
}

void TcpListener::deallocateClient(int client) {
	unsigned i = 1;
	
	while(m_master[i].fd != client) i++;
	available++;
	
	m_master[i].fd = 0;
}


void TcpListener::sendToClient(int clientSocket, const char* msg, int length) {
	send(clientSocket, msg, length, 0);
}

void TcpListener::broadcastToClients(int sendingClient, const char* msg, int length)
{
	// for (int i = 0; i < m_master.fd_count; i++) {
	for (int i = 1; i < available; i++) {
		// SOCKET outSock = m_master.fd_array[i];
		int outSock = m_master[i].fd;
		// if (outSock != m_socket && outSock != sendingClient) {
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
