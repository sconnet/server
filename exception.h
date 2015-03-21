//*****************************************************************************
//
// Copyright (C) 2001 Steve Connet.  All rights reserved.
//
// Source File Name : exception.h
// Author           : Steve Connet
//
// Version          : $Id: $
//
// File Overview : Used for throwing exceptions
//
// Revision History : 
//
// $Log: $
//
//*****************************************************************************

#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <string>
#include <ostream>

class Exception
{
public:
  Exception(const std::string& reason, int line, const std::string& file) :
    reason(reason),
    line(line),
    file(file) {}

  friend std::ostream& operator<<(std::ostream& out, const Exception& exception);

private:  
  std::string reason;
  int line;
  std::string file;
};

#endif // _EXCEPTION_H_
