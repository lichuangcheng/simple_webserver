/********************************************************************
*       (c) Copyright 2003, Hogeschool voor de Kunsten Utrecht
*                       Hilversum, the Netherlands
*********************************************************************
*
* File name     : sockbuf.h
* System name   : signature
* 
* Version       : $Revision: 1.4 $
*
*
* Description   : Definition of sockbuf class
*
*
* Author        : Marc_G
* E-mail        : marcg@dinkum.nl
*
*
********************************************************************/

/************
   $Log: sockbuf.h,v $
   Revision 1.4  2003/08/28 13:22:10  marcg
   adaptations for gcc-3

   Revision 1.3  2003/03/10 13:16:28  marcg
   overriding in_avail()

   Revision 1.2  2003/02/01 20:14:29  marcg
   Indication of client or server role

   Revision 1.1.1.1  2003/01/30 21:46:09  marcg
   Imported using TkCVS

*************/

#ifndef _SOCKBUF_H_
#define _SOCKBUF_H_

using namespace std;

#include <iostream>

#include <socket.h>


#define SOCKBUFSIZE 256

#define NO_ROLE		0
#define SERVER_ROLE	1
#define CLIENT_ROLE	2


class sockbuf : public streambuf , public Socket
{
public:
  sockbuf(int port);			// server
  sockbuf(string hostname,int port);	// client
  ~sockbuf();

  int underflow();	// get data from socket into get area
  int overflow(int c);	// put data from put area into socket
  int sync();		// flush pbuf

  int in_avail(void);

private:
  char gbuf[SOCKBUFSIZE];
  char pbuf[SOCKBUFSIZE];
  Socket s;
  int role;
};


#endif // _SOCKBUF_H_
