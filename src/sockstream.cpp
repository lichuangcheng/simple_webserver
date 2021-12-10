/********************************************************************
 *       (c) Copyright 2003, Hogeschool voor de Kunsten Utrecht
 *                       Hilversum, the Netherlands
 *********************************************************************
 *
 * File name     : sockstream.cpp
 * System name   : signature
 *
 * Version       : $Revision: 1.4 $
 *
 *
 * Description   : Implementation of sockstream class
 *
 *
 * Author        : Marc_G
 * E-mail        : marcg@dinkum.nl
 *
 *
 ********************************************************************/

/************
   $Log: sockstream.cpp,v $
   Revision 1.4  2003/03/10 13:16:52  marcg
   overriding in_avail()

   Revision 1.3  2003/02/07 16:57:07  marcg
   contains experiment with wouldblock()

   Revision 1.2  2003/02/01 20:16:11  marcg
   Fixes to make server and client use the correct socket for reading/writing

   Revision 1.1.1.1  2003/01/30 21:46:09  marcg
   Imported using TkCVS

*************/

#include <iostream>

#include <sockbuf.h>
#include <sockstream.h>

sockbuf::sockbuf(int port) // open as server
{
    role = NO_ROLE;
    Socket::create(); // should do some error checking on these calls...
    Socket::bind(port);
    Socket::listen();
    Socket::accept(s);
    setg(gbuf, gbuf + SOCKBUFSIZE, gbuf + SOCKBUFSIZE); // set get area
    setp(pbuf, pbuf + SOCKBUFSIZE);                     // set put area
    role = SERVER_ROLE;
    // s.set_non_blocking(true);
}

sockbuf::sockbuf(string host, int port) // open as client
{
    role = NO_ROLE;
    Socket::create();
    Socket::connect(host, port);
    setbuf(pbuf, SOCKBUFSIZE);
    setg(gbuf, gbuf + SOCKBUFSIZE, gbuf + SOCKBUFSIZE); // set get area
    setp(pbuf, pbuf + SOCKBUFSIZE);                     // set put area
    role = CLIENT_ROLE;
    // Socket::set_non_blocking(true);
}

sockbuf::~sockbuf()
{
    sync();
}

int sockbuf::underflow() // get data from socket into get area
{
    int bytes_read = 0;

    if (role == SERVER_ROLE)
    {
        bytes_read = s.read(gbuf, SOCKBUFSIZE);
    }
    else if (role == CLIENT_ROLE)
    {
        bytes_read = read(gbuf, SOCKBUFSIZE);
    }

    /*
      if(role==SERVER_ROLE){
        if(!s.wouldblock()){
          bytes_read = s.read(gbuf,SOCKBUFSIZE);
        }
      }
      else if(role==CLIENT_ROLE){
        if(!wouldblock()){
          bytes_read = read(gbuf,SOCKBUFSIZE);
        }
      }
    */

    else
        return -1;
    if (bytes_read <= 0)
        return -1;
    setg(gbuf, gbuf, gbuf + bytes_read); // set get area pointers
    return 0;
}

int sockbuf::overflow(int c) // put data from put area into socket
{
    int bytes_written;

    if (c == EOF)
        return sync();
    if (role == SERVER_ROLE)
        bytes_written = s.write(pbuf, SOCKBUFSIZE);
    else if (role == CLIENT_ROLE)
        bytes_written = write(pbuf, SOCKBUFSIZE);
    else
        return -1;
    setp(pbuf, pbuf + bytes_written); // set put area pointers
    *pptr() = c;                      // write out the last one
    pbump(1);                         // advance put pointer
    return c;                         // no idea why
}

int sockbuf::sync()
{
    // check if there's data to flush
    if (pptr() > pbase())
    {
        if (role == SERVER_ROLE)
            s.write(pbuf, pptr() - pbuf);
        else if (role == CLIENT_ROLE)
            write(pbuf, pptr() - pbuf);
        else
            return -1;
    }
    setp(pbuf, pbuf + SOCKBUFSIZE); // set put area
    return 0;
}

int sockbuf::in_avail(void)
{
    if (role == SERVER_ROLE)
    {
        if (s.wouldblock())
            return 0;
        else
            return 1;
    }
    else if (role == CLIENT_ROLE)
    {
        if (wouldblock())
            return 0;
        else
            return 1;
    }
    return 0;
    // return gptr() - eback();
}
