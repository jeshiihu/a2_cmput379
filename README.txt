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
	"User Ioanis: goodmorning everyone!"

To list the current users, type into the terminal (as a client): -listUsers, this message will not be sent to other clients


====== Killing/ exiting ======
To kill the server: kill <process id>
	a log file is created is the working directory server379processid.log (note: the processid is the actual id)
To kill the client: ctrl+c