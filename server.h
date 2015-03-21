//*****************************************************************************
//
// Copyright (C) 2001 Steve Connet.  All rights reserved.
//
// Source File Name : server.h
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

#ifndef _SERVER_H_
#define _SERVER_H_

#include <vector>
#include "exception.h"
#include "mutex.h"

typedef bool (*Callback)(int, void*);

class Server
{
 public:
  explicit Server(int port = 0, Callback fn = 0, void* data = 0);
  ~Server();

  int listen(int port = 0, Callback fn = 0, void* data = 0) throw (Exception);
  int select() throw (Exception);
  void quit();
  void setCallback(Callback fn, void* data) {
    callbackFn = fn;
    callbackData = data;
  }
  
 private:
  int fd;
  int port;
  bool listening;

  Callback callbackFn;
  void* callbackData;

  std::vector<int> client;
  Mutex mutex;
};


#endif // _SERVER_H_
