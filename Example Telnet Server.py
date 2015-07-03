import SocketServer

class MyTCPHandler(SocketServer.BaseRequestHandler):

    def handle(self):
        # self.request is the TCP socket connected to the client
        self.data = self.request.recv(1024).strip()
        print "{} wrote:".format(self.client_address[0])
        print self.data
        """
        SQL STUFF HERE

        if(self.data == SQL NAME or something)
        {
        self.request.sendall("g")//send good
        //INSERT TIME AND LOCATION INTO SQL HERE

        {
        else
        {
        self.request.sendall("bb")//send bad
        //DO NOTHING just send to arduino bad data
        }
        """
        self.request.sendall("g")

if __name__ == "__main__":
    HOST, PORT = "", 23
    server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)
    server.serve_forever()
