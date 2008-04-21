// Copyright (C) 2008 Simon Pantzare
// See COPYING for details.

#include "server.h"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include <pthread.h>

StreamServer::StreamServer(int port) throw(StreamServerError)
: port(port), notRunning(~0), thread(NULL)
{
        TaskScheduler* scheduler = BasicTaskScheduler::createNew();
        env = BasicUsageEnvironment::createNew(*scheduler);
        rtsp = RTSPServer::createNew(*env, port, NULL, 45);
        if (!rtsp)
        {
                throw StreamServerError();
        }
}

StreamServer::~StreamServer()
{
        Medium::close(rtsp);
        env->reclaim();
}

typedef MPEG1or2VideoFileServerMediaSubsession MPEGVidOnly;
typedef MPEG1or2FileServerDemux MPEG;
typedef MP3AudioFileServerMediaSubsession MP3;

#define DO_ADD_CHECKS() \
        if (!name) \
        { \
                name = file; \
        } \
        ServerMediaSession* look = rtsp->lookupServerMediaSession(name); \
        if (isRunning() && look != NULL) \
        { \
                throw StreamServerRunError(); \
        } \
        else \
        { \
                remove(name); \
        }

void StreamServer::addMP3(char const* file, char const* name = NULL)
throw(StreamServerRunError)
{
        DO_ADD_CHECKS();

        unsigned char interleaveCycle[] = {0,2,1,3};
        unsigned const interleaveCycleSize = 
                sizeof(interleaveCycle)/sizeof(unsigned char);
        Interleaving* interleaving;

        interleaving = new Interleaving(interleaveCycleSize, interleaveCycle);
        ServerMediaSession* sms = NULL;
        sms = ServerMediaSession::createNew(*env, name, name);
        sms->addSubsession(MP3::createNew(*env, file, False, False, 
                                          interleaving));
        rtsp->addServerMediaSession(sms);
}

void StreamServer::addMPEGVideo(char const* file, char const* name)
throw(StreamServerRunError)
{
        DO_ADD_CHECKS();

        ServerMediaSession *sms = NULL;
        sms = ServerMediaSession::createNew(*env, name, name);
        sms->addSubsession(MPEGVidOnly::createNew(*env, file, False, False));
        rtsp->addServerMediaSession(sms);
}

void StreamServer::addMPEG(char const* file, char const* name)
throw(StreamServerRunError)
{
        DO_ADD_CHECKS();

        ServerMediaSession *sms = NULL;
        sms = ServerMediaSession::createNew(*env, name, name);
        MPEG* demux = MPEG::createNew(*env, file, False);
        sms->addSubsession(demux->newVideoServerMediaSubsession());
        sms->addSubsession(demux->newAudioServerMediaSubsession());
        rtsp->addServerMediaSession(sms);
}

void StreamServer::remove(char const* name) throw(StreamServerRunError)
{
        ServerMediaSession* sms = NULL;
        sms = rtsp->lookupServerMediaSession(name);
        if (sms != NULL)
        {
                if (isRunning())
                {
                        throw StreamServerRunError();
                }
                else
                {
                        rtsp->removeServerMediaSession(sms);
                }

        }
}

void* StreamServer::listenClose(void* instance) // Will run in its own thread.
{
        StreamServer* inst = (StreamServer*)instance;
        inst->env->taskScheduler().doEventLoop(&(inst->notRunning));
        return NULL;
}

void StreamServer::run() throw(StreamServerRunError)
{
        if (notRunning)
        {
                notRunning = 0;
                pthread_create(&thread, NULL, StreamServer::listenClose, (void*)this);
        }
        else
        {
                throw StreamServerRunError();
        }
}

void StreamServer::stop() throw(StreamServerRunError)
{
        if (notRunning)
        {
                throw StreamServerRunError();
        }
        else
        {
                notRunning = ~0;
                // TODO: This join does not seem to finish if there is no
                //       client activity. 
                //pthread_join(thread, NULL);
        }
}

bool StreamServer::isRunning() const
{
        return ~notRunning;
}


const char* StreamServer::getURL(char const* name) const 
throw(StreamServerNameError)
{
        ServerMediaSession* sms = NULL;
        sms = rtsp->lookupServerMediaSession(name);

        if (sms == NULL)
        {
                throw StreamServerNameError();
        }
        return rtsp->rtspURL(sms);
}

///////////////////////////////////////////////////////////////////////////////
// TESTS
int main()
{
        StreamServer stream;

        // This is how we add files to the streaming server. Both these files
        // should exist in our working directory.
        stream.addMP3("m1.mp3", "test_mp3");
        stream.addMP3("m2.mp3", "test_mp3_2");
        stream.addMPEGVideo("test.mpg", "test_mpg");

        // Just add and remove a stream. Behaviour when trying to stream
        // nonexisting files is undefined.
        stream.addMP3("", "badmp3");
        stream.remove("badmp3");

        // This should work.
        stream.getURL("test_mp3");
        stream.getURL("test_mpg");
        stream.run();
        stream.stop();

        // Try to remove a stream that does not exist. Nothing should happen
        // even if we are running or not.
        stream.remove("bad"); 
        stream.run();
        stream.remove("bad");
        
        // However, trying to remove a stream that exists while the server is
        // running is forbidden.
        try
        {
                stream.remove("test_mp3_2");
        }
        catch (StreamServerRunError) {}
        stream.stop();

        // Now we can remove it.
        stream.remove("test_mp3_2");

        // We can not add a new stream that has a name that already exists in
        // our database when the server is running.
        stream.run();
        try
        {
                stream.addMP3("m2.mp3", "test_mp3");
        }
        catch (StreamServerRunError) {}

        // If the name does not exist in the database, it is OK even if the 
        // server is running.
        stream.addMP3("m1.mp3", "test_mp3_3");

        // When the server is not running we can overwrite old stream names.
        stream.stop();
        stream.addMP3("m2.mp3", "test_mp3");

        // A StreamServerNameError exception should be thrown when trying to 
        // get the URL for streams that has not been added.
        try
        {
                stream.getURL("bad");
        }
        catch (StreamServerNameError) {}

        // We can not start the server more than once in a row (same when 
        // stopping).
        stream.run();
        try
        {
                stream.run();
        }
        catch (StreamServerRunError) {}
        stream.stop();
        try
        {
                stream.stop();
        }
        catch (StreamServerRunError) {}

        return 0;
}
