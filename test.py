# -*- coding: utf-8 -*-
# Copyright (C) 2008 Simon Pantzare
# See COPYING for details.

import streamserver
import unittest

class NameGenerator:
    counter = 0

    def new(self):
        self.counter += 1
        return str(self.counter)
    
    def last(self):
        return str(self.counter)


class TestStreamServer(unittest.TestCase):
    s = streamserver.StreamServer()
    name = NameGenerator()

    def setUp(self):
        pass

    def tearDown(self):
        if self.s.isRunning():
            self.s.stop()

    def testStartAndStop(self):
        "Start and stop the server on the default port"
        self.s.run()
        self.s.stop()

    def testTwoInstancesSamePort(self):
        "Try to create two instances on the same port"
        self.assertRaises(streamserver.StreamServerError, 
                          lambda: streamserver.StreamServer())

    def testTwoInstancesNotSamePort(self):
        "Create two instances on different ports and start and stop them"
        s1 = streamserver.StreamServer(8555)
        self.s.run(); s1.run(); self.s.stop(), s1.stop()

    def testAddAndRemoveManyFiles(self):
        "Add and remove many files to a server when it is not running"
        names = [self.name.new() for dummy in xrange(1000)]
        for name in names:
            self.s.addMP3("m1.mp3", name)
        for name in names:
            self.s.remove(name)

    def testRemoveNonexisting(self):
        """Remove a stream that does not exist from server.
        No exception is raised when we delete streams that does not exist
        on server."""
        self.s.remove(self.name.new())

    def testRemoveStreamWhenRunning(self):
        """Remove streams from server when it is running.
        No exception is raised when we delete streams that does not exist
        on server."""
        # First, we add a file to the server
        self.s.addMP3("m1.mp3", self.name.new())

        # Then we start the server and try to remove this file, this
        # should raise an exception
        self.s.run()
        self.assertRaises(streamserver.StreamServerRunError,
                          lambda: self.s.remove(self.name.last()))

        self.s.remove(self.name.new())
        self.s.stop()

    def testAddFiles(self):
        "Add files to server"
        # Add files when we are not running
        self.s.addMP3("m1.mp3", self.name.new())
        self.s.addMPEG("va1.mpg", "bar")
        self.s.addMPEGVideo("vidonly1.mpg", "baz")

        # We can add new streams to the server when it is running. There must 
        # not exist a stream with the same name though.
        self.s.run()
        self.s.addMP3("m1.mp3", self.name.new())

        # Try to add a stream with an occupied name when we are running
        self.assertRaises(streamserver.StreamServerRunError,
                          lambda: self.s.addMP3("m2.mp3", self.name.last()))

        # We can reuse names when the server is not running
        self.s.stop()
        self.s.addMP3("m1.mp3", self.name.last()) 

    def testStartTwice(self):
        "It is not allowed to start or stop a server twice in a row"
        self.s.run()
        self.assertRaises(streamserver.StreamServerRunError,
                          lambda: self.s.run())
        self.s.stop()
        self.assertRaises(streamserver.StreamServerRunError,
                          lambda: self.s.stop())


if __name__ == '__main__':
    unittest.main()

