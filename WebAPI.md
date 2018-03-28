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
{"action":"query", "type":"version"} | "18w11a" | Query current server version
||
{"action":"query", "type":"players"} | {"online":1, "max":20}<br>{} _(If server is not ready)_ | Query player list<br>* `max` may be 0 if regression expression matching failed
{"action":"query", "type":"players", "list":true} | {"online":1, "max":20, "list":["zhiiyb"]}<br>{} _(If server is not ready)_ | Query player list
||
{"action":"query", "type":"backup"} | {"status":"idle", "last":1522269049}<br>{"status":"scheduled", "time":1522269064, "last":1522269049}<br>{"status":"active", "last":1522269049} | Query backup status<br>* `time` and `last` are `int64` values<br>* `last` may not be available
||
{"action":"query", "type":"restart"} | {"status":"stopped"}<br>{"status":"starting"}<br>{"status":"running"}<br>{"status":"pending"}<br>{"status":"scheduled", "time":1522269049} | Query server status<br>Stopped: `stopped` / `scheduled`<br>Running: `starting` / `running` / `pending`<br>* `time` is an `int64` value

WebSockets
==========

ws://host:port

Protocol | Description
--|--
web-console | Forwards input and output to/from the `servmc` application
