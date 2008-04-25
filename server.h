// Copyright (C) 2008 Simon Pantzare
// See COPYING for details.

#ifndef SERVER_H
#define SERVER_H

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

class StreamServerError {};
class StreamServerRunError {};
class StreamServerNameError {};

class StreamServer
{
        public:
                StreamServer(int port = 8544) 
                        throw(StreamServerError);

                ~StreamServer();

                void addMP3(char const* file, char const* name)
                        throw(StreamServerRunError);

                void addMPEGVideo(char const* file, char const* name)
                        throw(StreamServerRunError);

                void addMPEG(char const* file, char const* name)
                        throw(StreamServerRunError);

                void run() 
                        throw(StreamServerRunError);

                void stop() 
                        throw(StreamServerRunError);

                void remove(char const* name) 
                        throw(StreamServerRunError);

                const char* getURL(char const* name) const
                        throw(StreamServerNameError);

                bool isRunning() const;

        private:
                const int port;
                RTSPServer* rtsp;
                UsageEnvironment* env; 
                char notRunning;
                pthread_t thread;
                static void* listenClose(void* inst);
};

#endif
