Requirement:
    Please write 2 Linux applications that act as client and server processes and comply to following requirement:
        Communication method between client and server should be 1 of following IPC methods (your choice): message queue, socket 
        We should allow multiple client processes runing at same time but only 1 server process 
        In server process, it should maintain a list of clients that are connected to it and their statuses ( LOGGED ON or LOGGED ON ) 
        There should be following kind of communications between clients and server
            Connect/disconnect 
                Server should be able to tell if a client is connected/disconnected and add/remove it to/from its client list.
            Message exchanging
                Logon:
                    - Client sends 'LOGON' to Server 
                    - Server changes that client status in its client list to LOGGED_ON
                    - Server sends back 'OK' to Client
                Logoff:
                    - Client sends 'LOGOFF' to Server 
                    - Server changes that client status in its client list to LOGGED_OFF
                    - Server sends back 'OK' to Client            
                Status request:
                    - Client sends 'STATUS_REQ' to Server
                    - Server checks that client status in its client list 
                    - Server sends back either 'ON' or 'OFF' status to Client 
        Client process should should automatically connect to server upon startup and provide Command Line Interface (CLI) to user with following options (corresponding to above message flows ):
            - Send logon to server 
            - Send logoff to server 
            - Get status 
            - Exit ( disconnect and terminate the process )
        Server process should start accepting connection from clients upon startup and provide CLI to user with followng options:
            - Print client statuses: 
                - Number of connected clients
                - Number of logged on clients
                - Number of logged off clients 
            - Exit ( terminate the process )
 
-- Build and Run
make		// build new server and client
make clean	// remove old