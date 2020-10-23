# OS_Assignment4

i used the code from date_server and only edited the client helper method
run the server: ./server 127.0.0.1 5001
than open up firefox and type in 127.0.0.1:5001

- This project was a bitch to learn, just because of parsing in C
- Now uses a static header and a buffer to write multiple times, so we now use constant memory
      - Use static header
      - Transfer-Encoding: chunked so that the client's browser waits for </html> before the page loads
      - We don't need to specify Content-Length because of that, so we can loop write() with a static buffer. O(1) memory!
- Parses HTTP request:
      - Split request by whitespace
      - Check if firstToken is GET
      - Check if secondToken[0] is '/' for a valid directory
      - If length of secondToken is 1, there is only a '/' -> "index.html" as default
      - ask server to open secondToken as a filepath

## To Do
- I'm using Docker, and external sockets are not doable. Working on getting dev tools to use a feature that might let me do this in the future. For now, test it with your browser to make sure it works
- Add signal to main to close the socket when a user ctrl+c's the server. Currently stopping the server without closing it from telnet first will forever occupy the address/port (or until restart)
- Check over the assignment instructions in case we missed something. And check the browser!

VERY  helpful link
https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa
