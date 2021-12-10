/********************************************************************
 *       (c) Copyright 2003, Hogeschool voor de Kunsten Utrecht
 *                       Hilversum, the Netherlands
 *********************************************************************
 *
 * File name     : sockstream_main.cpp
 * System name   : signature
 *
 * Version       : $Revision: 1.4 $
 *
 *
 * Description   : main() for client-server tests using sockstream
 *
 *
 * Author        : Marc_G
 * E-mail        : marcg@dinkum.nl
 *
 *
 ********************************************************************/

/************
   $Log: sockstream_main.cpp,v $
   Revision 1.4  2005/04/18 10:31:07  marcg
   Modifications for college

   Revision 1.3  2003/03/10 13:17:34  marcg
   using in_avail()
   source is becoming a bit of a mess

   Revision 1.2  2003/02/07 16:58:37  marcg
   create independent input- and output streams
   Full-duplex still doesn't work though...

   Revision 1.1.1.1  2003/01/30 21:46:09  marcg
   Imported using TkCVS

*************/

#include <iostream>

#include <fcntl.h>   // threads
#include <pthread.h> // threads of course

#include <sockbuf.h>
#include <sockstream.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static sockbuf *sb;
static iostream *mystream;

int server;

void *input_thread(void *arg)
{
    char c;

    while (true)
    {

        pthread_mutex_lock(&mutex);
        int available = sb->in_avail();
        if (available > 0)
            *mystream >> c; // read from get area
        pthread_mutex_unlock(&mutex);

        if (available == 0)
        {
            usleep(10000);
            continue;
        }

        cout << c;
    }
    cout << "INPUT ends\n";
    return 0;
}

void *output_thread(void *arg)
{
    char c = 'a';

    while (true)
    {
        cin >> c;

        pthread_mutex_lock(&mutex);
        *mystream << c;
        mystream->flush();
        sb->sync();
        mystream->sync();
        pthread_mutex_unlock(&mutex);
    }
    cout << "OUTPUT ends\n";
    return 0;
}

int main(int argc, char **argv)
{
    pthread_t thread_1, thread_2;
    void *retval;

    if (string(argv[0]) == "streamserver")
    {
        sb = new sockbuf(7777); // server
        server = 1;
    } // server
    else
    {
        sb = new sockbuf("localhost", 7777); // client
        server = 0;
    }

    mystream = new iosockstream(sb);
    mystream->unsetf(ios::skipws);
    cin.unsetf(ios::skipws);

    // Create two threads
    if (pthread_create(&thread_1, NULL, output_thread, NULL) ||
        pthread_create(&thread_2, NULL, input_thread, NULL))
    {
        cout << "Thread creation fails\n";
        exit(1);
    }

    // Join (collapse) the threads
    if (pthread_join(thread_1, &retval) ||
        pthread_join(thread_2, &retval))
    {
        cout << "Thread join fails\n";
    }

    return 0;
}
