/************************************************************************
 *   Copyright (C) Mark Thomas <markbt@efaref.net>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef Y_CPP_Y_H
#define Y_CPP_Y_H

#include <Y/const.h>
#include <Y/c++/message.h>

#include <sys/poll.h>
#include <sys/time.h>
#include <inttypes.h>

#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <set>
#include <map>

#include <pthread.h>

enum YListenMode
{
  Y_LISTEN_READ = 1<<0,
  Y_LISTEN_WRITE = 1<<1,
  Y_LISTEN_EXCEPT = 1<<2,
};

namespace Y
{
  class Class;
  class Object;
  class Reply;
  class Timer;

  /** \brief Connection to a Y server
   * \ingroup comm
   */
  class Connection
  {
    friend class Reply;
    friend class Timer;

  public:
    Connection ();
    virtual ~Connection ();

    void run ();
    void runThreaded ();
    void poll (int timeout);
    void stop ();

    Class *findClass (std::string className);

    Reply *sendMessage (const Message *);

    void registerFD (int fd, int mask, void *data, void (*call)(int, int, void *));
    void unregisterFD (int fd);
    void changeFD (int fd, int mask);

    void detachReply (uint32_t seq);

    void createdObject (Object *obj);
    Object *findObject (uint32_t oid);
    void destroyObject (uint32_t oid);

  private:

    class TimeEvent
    {
      friend class Y::Connection;
    public:
      TimeEvent (struct timeval tv_, Y::Timer* timer_) : tv(tv_), timer(timer_) {}
      bool operator<(TimeEvent ev) const
      {
        /* First we sort by time, earliest first
         */
        if (tv.tv_sec != ev.tv.tv_sec)
          return tv.tv_sec < ev.tv.tv_sec;
        if (tv.tv_usec != ev.tv.tv_usec)
          return tv.tv_usec < ev.tv.tv_usec;
        /* Then we sort (arbitrarily) by timer pointer
         *
         * Note that there should only be one TimeEvent per Timer
         */
        return timer < ev.timer;
      }
      bool ready(struct timeval now) const
      {
        if (tv.tv_sec != now.tv_sec)
          return tv.tv_sec < now.tv_sec;
        return tv.tv_usec < now.tv_usec;
      }
    private:
      struct timeval tv;
      Y::Timer* timer;
    };

    std::set<TimeEvent> timers;
    pthread_mutex_t timers_mutex;

    TimeEvent* setTimer (int msec, Y::Timer* timer);
    void unsetTimer (TimeEvent* event);

    bool debug_messages, debug_io;

    std::list<Message *> messages;
    pthread_mutex_t messages_mutex;

    std::map<uint32_t, Reply*> replies;
    pthread_mutex_t replies_mutex;

    std::map<uint32_t, Object*> objects;
    pthread_mutex_t objects_mutex;

    std::map<std::string, Class*> classes;
    pthread_mutex_t classes_mutex;

    void processMessage (Message *);

    class FDHandler
    {
    public:
      FDHandler () : fd(-1), mask(0), data(NULL), callback(NULL) {}
      FDHandler (int fd_, int mask_, void *data_, void (*callback_)(int, int, void *))
        : fd(fd_), mask(mask_), data(data_), callback(callback_)
        {}
          
      int fd;
      int mask;
      void *data;
      void (*callback)(int, int, void *);
    };

    typedef std::map<int, FDHandler> fdmap;
    fdmap fds;
    pthread_mutex_t fds_mutex;

    std::string inbound_buffer;
    std::string outbound_buffer;
    pthread_mutex_t inbound_mutex;
    pthread_mutex_t outbound_mutex;

    void tcpInitialise(const char *display);
    void unixInitialise(const char *display);
    void setNonBlocking(int fd);

    void unbufferMessages ();
    void doPollIO(struct pollfd *ufds, int timeout);
    void doPoll(int poll_timeout);
    void doDispatch();

    static void *dispatchThreadLauncher(void *arg);

    void stopThreaded();
    void pollThreadMain();
    void dispatchThreadMain();

    pthread_t dispatch_thread;
    pthread_mutex_t dispatch_mutex;
    pthread_cond_t dispatch_cond;

    void readServer();
    void writeServer();

    /* We have two because poll() needs something to update as it
     * works, and this avoids having to allocate for every poll()
     * call - we just memcpy(pollfd_list, working_pollfd_list, ...) 
     * each time
     */
    int pollfds;
    struct pollfd *pollfd_list, *working_pollfd_list;
    void updateFDList ();
    pthread_mutex_t pollfd_list_mutex;

    int server_fd;

    /* When state is none:
     *
     *  run() changes to running
     *  runThreaded() changes to threaded
     *  poll() changes to polling
     *
     * When state is running:
     *
     *  run() aborts
     *  runThreaded() aborts
     *  poll() polls once and returns
     *
     * When state is threaded:
     *
     *  run() aborts
     *  runThreaded() aborts
     *  poll() does nothing
     *
     * When state is polling:
     *
     *  run() aborts
     *  runThreaded() aborts
     *  poll() polls once and returns
     *
     * That should be safe for all sequences
     */
    pthread_mutex_t state_mutex;
    pthread_cond_t state_cond;
    enum {none, running, threaded, polling} state;
    bool stopping;
  };
}

#endif

/* arch-tag: 72c79697-f1f5-415d-aa8b-346cdbd07511
 */
