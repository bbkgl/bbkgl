#ifndef BBKGL_EVENTLOOP_H
#define BBKGL_EVENTLOOP_H

#include <boost/noncopyable.hpp>
#include <cassert>
#include "Thread.h"

namespace bbkgl
{
    class EventLoop : public boost::noncopyable
    {
    public:
        EventLoop();
        ~EventLoop();

        void loop();

        void assertInLoopThread()
        {

        }

        bool isInLoopThread() const { return threadId_ == CurrentThread::; }

    private:

        void abortNotInLoopThread();

        bool looping_;
        const pid_t threadId_;

    };
}


#endif //BBKGL_EVENTLOOP_H
