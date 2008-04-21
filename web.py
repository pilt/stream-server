# -*- coding: utf-8 -*-
# Copyright (C) 2008 Simon Pantzare
# See COPYING for details.

import cherrypy
from streamServer import StreamServer
from random import randint
from code import interact
import os.path

# Let us first set up the streaming server and add some media to it.
stream = StreamServer()
media = [("m1.mp3", "Sample_MP3_1"),
         ("m2.mp3", "Sample_MP3_2"),
         ("va1.mpg", "Sample_MPEG"),
         ("vidonly1.mpg", "Sample_MPEG_video_only_1"),
         ("vidonly2.mpg", "Sample_MPEG_video_only_2")]
for mp3 in range(0, 2):
    stream.addMP3(*media[mp3])
stream.addMPEG(*media[2])
for mpg in range(3, len(media)):
    stream.addMPEGVideo(*media[mpg])

# We could use a templating language but I think the extra dependencies is not
# worth it in this case.

html = """
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
   "http://www.w3.org/TR/html4/strict.dtd">
<html lang="en">
<head><meta http-equiv="content-type" content="text/html; charset=utf-8">
    <title>Streaming Server Demo</title>
    <script src="http://prototypejs.org/assets/2008/1/25/prototype-1.6.0.2.js"
        type="text/javascript"></script>
    <script type="text/javascript">
        window.onload = function()
        {
            new Ajax.PeriodicalUpdater('media', '/updateMedia', 
                { method: 'get', frequency: 3 });
        }
    </script>
    <style type="text/css">
    body {
        font-family: verdana, arial, sans-serif;
        }
    #media {
        border: 1px solid #000; 
        padding: 1em;
        }
    </style>
</head>
<body>

<h1>Play Media</h1>
<div id="media"> </div>
</body>
</html>
"""

class Main:
    @cherrypy.expose
    def default(self):
        return html

    @cherrypy.expose
    def updateMedia(self):
        """Get HTML links to the streaming server's media. If the streaming 
        server is not running we will return a message 'Streaming server is 
        not running'."""
        if not stream.isRunning():
            return "<p>Streaming server is not running.</p>"
        else:
            html = "<ul"
            for m in media:
                html += "><li<a href=\"%s\">%s</a></li\n" \
                    % (stream.getURL(m[1]), m[1])
            return html + "></ul>"

def run():
    stream.run()
    cherrypy.config.update(os.path.join(os.path.dirname(__file__), 'web.conf'))
    cherrypy.tree.mount(Main())
    cherrypy.server.quickstart()
    cherrypy.engine.start_with_callback(lambda: interact(local=globals()))
    stream.stop()

if __name__ == '__main__':
    run()

