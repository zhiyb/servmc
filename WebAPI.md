GET requests
============

http://host:port/

Simple HTTP server, supplies documents from the `www` directory

POST requests
=============

http://host:port/q

Content type: application/json

Request | Return | Description
--|--|--
{"action":"query","type":"version"} | "18w11a" | Query current server version

WebSockets
==========

ws://host:port

Protocol | Description
--|--
web-console | Forwards input and output to/from the `servmc` application
