//*****************************************************************************
//
// Copyright (C) 2001 Steve Connet.  All rights reserved.
//
// Source File Name : main.cpp
// Author           : Steve Connet
//
// Version          : $Id: $
//
// File Overview : test program for my server library
//
// Revision History : 
//
// $Log: $
//
//*****************************************************************************

#include <iostream>

#include <signal.h>
#include <unistd.h>

#include "server.h"

using namespace std;

//
//-------------------------------------------------------------------------
// Method         : operator<<
//
// Implementation : display the exception to user's stream
// 
//-------------------------------------------------------------------------
//
ostream& operator<<(std::ostream& out, const Exception& exception)
{
  std::string sep(79, '*');
  out << sep
      << "\nexception: " << exception.reason
      << "\n    error: " << errno << " (" << strerror(errno) << ")"
      << "\n     line: " << exception.line
      << "\n     file: " << exception.file
      << '\n' << sep;
  return out;

} // operator<<

//
//-------------------------------------------------------------------------
// Method         : signalHandler
//
// Implementation : Handle signals so we can gracefully exit
// 
//-------------------------------------------------------------------------
//
void signalHandler(int signo)
{
  cout << "got signal " << signo << endl;
  
} // signalHandler

//
//-------------------------------------------------------------------------
// Method         : read_client
//
// Implementation : callback function called by the server object
//                  when there is data on the socket to read
//
//                  return false if client disconnected or there
//                  was an error reading from the client
// 
//-------------------------------------------------------------------------
//
bool read_client(int fd, void* data)
{
  const int buf_size = 8192; // 8k
  char buf[buf_size + 1];

  memset (buf, 0, buf_size);
  int bytes = read (fd, (char*)buf, buf_size);
  if (bytes <= 0)
    return false;

  // write this to a file
  cout << buf << endl;

  return true;
  
} // read_client

//
//-------------------------------------------------------------------------
// Method         : main
//
// Implementation : program entry point
// 
//-------------------------------------------------------------------------
//
int main(int argc, char* argv[])
{
  // setup signals to catch
  signal(SIGTERM, signalHandler);
  signal(SIGINT, signalHandler);
  signal(SIGPIPE, SIG_IGN); 

  Server server;
  try {
    
    server.listen(32000, read_client);
    server.select();
  }
  catch(Exception exception) {

    cerr << exception << endl;
  }
  catch(...) {

    cerr << "Unknown exception caught!" << endl;
  }
  
  return 0;

} // main
