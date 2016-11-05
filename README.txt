# a2_cmput379

Documentation:


====== Running ======
To run the server: ./server379 <portnumber>
To run the client: ./chat379 <hostnumber> <portnumber> <username>


====== In Chat ======
Once joined as a client, the client receives the current number of users, followed by the list of the usernames. The user will see that they have just joined. Any users that join or disconnect will be notified
	"User Chris: joined the server!"
	"User Ben: disconnected from the server."
Messages from everyone shows as
	"User Dr.Nikolaidis: goodmorning everyone!"

To list the current users, type into the terminal (as a client): -listUsers
	- this message will not be sent to other clients


====== Killing/ exiting ======
To kill the server: kill <processid>
	a log file is created is the working directory server379processid.log (note: the processid is the actual id)
To kill the client: ctrl+c


====== Notes ======
- The keep alive messages send every 30 seconds from a client, and timeout is implemented in the server (lines 456-470). However, the timeout unfortunately does not seem to work and doesn't disconnect a user if no keep alive is sent.

- the users lists are dynamically created (add and delete is also dynamic)

- for a message from a client (entered through the terminal), the max number of characters read and sent to all clients is limited to the first 256 characters

- an attempt was made to use forking in the server, however it didn't function properly due to the following issues:
	kept on getting blocked in some areas
	could not get sharing data through pipe to work
	tried fork when it got blocked, but failed.
the thought process behind using the fork in server:
	enters a while loop
	checks which if loop should be executed based on the pid of the processes
	if it is a parent pid:
		it checks if there is a new connection. If so, create a socket for it and a child processes to be dedicated to it
		if it recieves and messages from a child, broadcast it to all the other children
		send updates if any to all the children
	if it is a child pid:
		listen for any messages from the client and pipe it to the parent
		listen to any messages from the server and pipe it to the client

