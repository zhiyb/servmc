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
{"action":"query","type":"players"} | {"online":1,"max":20}<br>{} _(If server is not ready)_ | Query player list<br>* `max` may be 0 if regression expression matching failed
{"action":"query","type":"players","list":true} | {"online":1,"max":20,"list":["zhiiyb"]} | Query player list

WebSockets
==========

ws://host:port

Protocol | Description
--|--
web-console | Forwards input and output to/from the `servmc` application
