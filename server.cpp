//*****************************************************************************
//
// Copyright (C) 2001 Steve Connet.  All rights reserved.
//
// Source File Name : server.cpp
// Author           : Steve Connet
//
// Version          : $Id: $
//
// File Overview : Listens for incoming connections and calls a user's
//                 callback so the user can read the data.
//
//                 TODO: make this into a libary
//
// Revision History :
//
// $Log: $
//
//*****************************************************************************

#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h> // AF_INET, shutdown
#include <fcntl.h>      // fcntl
#include <netdb.h>      // gethostbyname
#include <arpa/inet.h>  // inet_ntoa
#include <unistd.h>     // close,read/write

#include <iostream>

#include "server.h"
#include "mutex.h"

using namespace std;

//
//-------------------------------------------------------------------------
// Method         : constructor
//
// Implementation : overrides default port
//
//-------------------------------------------------------------------------
//
Server::Server(int port, Callback fn, void *data) :
    fd(-1),
    port(port),
    listening(false),
    callbackFn(fn),
    callbackData(data)
{
} // constructor


//
//-------------------------------------------------------------------------
// Method         : destructor
//
// Implementation : clean up
//
//-------------------------------------------------------------------------
//
Server::~Server()
{
    quit();

} // destructor

//
//-------------------------------------------------------------------------
// Method         : listen
//
// Implementation : Starts listening for connections.
//
//-------------------------------------------------------------------------
//
int Server::listen(int port, Callback fn, void *data) throw(Exception)
{
    if(listening)
    {
        return 0;
    }

    // override port or callback
    if(port)
    {
        this->port = port;
    }
    if(fn)
    {
        callbackFn = fn;
    }
    if(data)
    {
        callbackData = data;
    }

    // create our listening socket
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        throw(Exception("socket creation failed", __LINE__, __FILE__));
    }

    // allow rebinding of this socket
    long l = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &l, sizeof(long));

    // bind to port and listen for connections
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons((short) port);
    if((::bind(fd, (struct sockaddr *) &server, sizeof(struct sockaddr_in)) == -1) || (::listen(fd, SOMAXCONN) == -1))
    {
        close(fd);
        throw(Exception("bind or listen failed", __LINE__, __FILE__));
    }

    listening = true;
    return 0;

} // listen

//
//-------------------------------------------------------------------------
// Method         : select
//
// Implementation : reads incoming data
//
//-------------------------------------------------------------------------
//
int Server::select() throw(Exception)
{
    fd_set read_set;
    FD_ZERO(&read_set);

    // keep reading data until we are told to stop
    while(!mutex.waitEvent(0))
    {

        // make sure users callback is still valid
        if(!callbackFn)
        {

            quit();
            throw(Exception("User callback function is not set", __LINE__, __FILE__));
        }

        mutex.lock();

        // setup for select
        int max_fd = fd;
        if(!client.empty())
        {
            max_fd = *max_element(client.begin(), client.end());
        }

        FD_SET(fd, &read_set);
        for(unsigned int n = 0; n < client.size(); n++)
        {
            FD_SET(client[n], &read_set);
        }

        // wait for incoming connection
        if(::select(max_fd + 1, &read_set, NULL, NULL, NULL) <= 0)
        {

            if(errno == EINTR)   // exit on signal
            {
                break;
            }
            else
            {
                throw(Exception("select failed", __LINE__, __FILE__));
            }
        }

        // check each fd for readability
        for(vector<int>::iterator cur(client.begin()), end(client.end()); cur != end; ++cur)
        {
            int client_fd = *cur;

            // call user's callback if fd is readable
            if(FD_ISSET(client_fd, &read_set))
            {
                FD_CLR(client_fd, &read_set);

                // call user's callback fn
                if(!callbackFn(client_fd, callbackData))
                {
                    shutdown(client_fd, SHUT_RDWR);
                    close(client_fd);
                    cur = client.erase(cur);
                    cerr << "-- client disconnected fd " << client_fd << endl;
                }
            }
        }

        // new connection?
        if(FD_ISSET(fd, &read_set))
        {
            FD_CLR(fd, &read_set);
            struct sockaddr_in acceptSock;
            socklen_t len = sizeof(struct sockaddr_in);
            int new_fd = ::accept(fd, (struct sockaddr *)&acceptSock, &len);
            if(new_fd == -1)
            {
                throw(Exception("Accept failed, returned -1", __LINE__, __FILE__));
            }
            else
            {
                client.push_back(new_fd);
                cerr << "-- client connected fd " << new_fd << endl;
            }
        }

        mutex.unlock();

    } // end while

    mutex.unlock();
    return 0;

} // select

//
//-------------------------------------------------------------------------
// Method         : quit
//
// Implementation : quits reading and listening for connections
//
//-------------------------------------------------------------------------
//
void Server::quit()
{
    mutex.signal();
    mutex.lock();

    // close listening socket
    close(fd);

    // close connected sockets
    for(unsigned int n = 0; n < client.size(); n++)
    {

        shutdown(client[n], SHUT_RDWR);
        close(client[n]);
        cerr << "-- client disconnected " << client[n] << endl;
    }

    client.clear();
    listening = false;

    mutex.reset();
    mutex.unlock();

} // quit
