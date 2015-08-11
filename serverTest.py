import SocketServer
import sys
from random import randint
from LTS import *


class MyTCPHandler(SocketServer.BaseRequestHandler):
    """
    The RequestHandler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """

    def handle(self):
        self.data = self.request.recv(1024).strip()
        print self.client_address
        print self.data
        if len(self.data) != 36:
            sys.exit()

        try:
            # Load learner
            learner = learnerfromRFID(self.data)
            learner.Location = randint(0,31)
            self.request.sendall("g")

        except Exception as e:
            print "SOMETHING BROKE"
            print e.message
            self.request.sendall("bb")

if __name__ == "__main__":
    print 'STARTING'
    HOST, PORT = '', 23

    # Create the server, binding to localhost on port 9999
    server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    server.serve_forever()
