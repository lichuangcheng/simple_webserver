/********************************************************************
*       (c) Copyright 2003, Hogeschool voor de Kunsten Utrecht
*                       Hilversum, the Netherlands
*********************************************************************
*
* File name     : sockstream.h
* System name   : signature
* 
* Version       : $Revision: 1.2 $
*
*
* Description   : Definition of sockstream class
*		  A derived class of iostream for implementation
*		  of socket-IO with iostream properties
*
*
* Author        : Marc_G
* E-mail        : marcg@dinkum.nl
*
*
********************************************************************/

/************
   $Log: sockstream.h,v $
   Revision 1.2  2003/08/28 13:22:10  marcg
   adaptations for gcc-3

   Revision 1.1.1.1  2003/01/30 21:46:09  marcg
   Imported using TkCVS

*************/

#ifndef _SOCKSTREAM_H_
#define _SOCKSTREAM_H_

using namespace std;

#include <iostream>

#include <sockbuf.h>

/*
class iosockstream : public iostream
{
public:
  iosockstream(sockbuf* sb) : iostream(sb), ios(sb) {}
};
*/

class iosockstream : public std::iostream
{
public:
  iosockstream(sockbuf* sb): std::iostream(sb) {}
};



#endif // _SOCKSTREAM_H_
