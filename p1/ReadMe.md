
CSC361 P1 ReadMe
Leanne Feng
V00825004

In command: 
1. type make to excute
2. type ./sws 8045 /tmp/www(depends on where you put index.html)  to enter the waiting mode
3. in client type: bash runme1st.sh , then click "q" to exit in server.


The sturucture of my code includes following major parts:

    	main function: CREATE socket and bind the IP address and Port, Configure socket to reuse ports, while loop: including all useful functions: select receiveform and sendto.
	
	printtime(): print out MMM DD HH:MM:SS
	
	requestCase(): return cases are: 200 if finds file else if format error return else not found file return 400
	
	uppercase():returns the string it recieved, only with all the letters in uppercase.
	compare():compare the format if correct format return true else false
	
	findfile(): find if the file exsit in the given path if does, open file and copy content into buffer else return case 404. 
	
	separate(): separate echo string whenever there is a space. for example, command[0] is GET etc.
	
    	
Bonus: added 1 more incorrect format request
	added 1 more textfile check request,where textfile is in new directory /tmp/www/new/lala.txt 

