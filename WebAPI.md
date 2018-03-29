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
{"action":"query", "type":"players"} | {"online":1, "max":20}<br>{} _(If server is not ready)_ | Query player list<br>* `max` may be 0 if regular expression matching failed
{"action":"query", "type":"players", "list":true} | {"online":1, "max":20, "list":["zhiyb"]}<br>{} _(If server is not ready)_ | Query player list<br>* `max` may be 0 if regular  expression matching failed
{"action":"query", "type":"backup"} | {"status":"idle", "last":1522269049}<br>{"status":"scheduled", "time":1522269064, "last":1522269049}<br>{"status":"active", "last":1522269049} | Query backup status<br>* `time` and `last` are `int64` values<br>* `last` may not be available
{"action":"query", "type":"restart"} | {"status":"stopped"}<br>{"status":"starting"}<br>{"status":"running"}<br>{"status":"pending"}<br>{"status":"scheduled", "time":1522269049} | Query server status<br>Stopped: `stopped` / `scheduled`<br>Running: `starting` / `running` / `pending`<br>* `time` is an `int64` value
{"action":"query", "type":"system"} | {"cpus":[<br>["cpu",3486,2752],<br>["cpu0",573,524],<br>["cpu1",581,572]]} | `cpus` will be an array of size *cpu_count+1*, the first one will be the total of all CPUs<br>Elements in `cpu` arrays: `name`, `total`, `id`<br>* `cpus` may not be available during startup
{"action":"query", "type":"system", "detailed":true} | {"cpus":[<br>["cpu",3486,682,0,48,2752,3,0,1,0],<br>["cpu0",573,41,0,7,524,0,0,1,0],<br>["cpu1",581,7,0,2,572,0,0,0,0]]} | `cpus` will be an array of size *cpu_count+1*, the first one will be the total of all CPUs<br>Elements in `cpu` arrays: `name`, `total`, `us`, `sy`, `ni`, `id`, `wa`, `hi`, `si`, `st`<br>* `cpus` may not be available during startup

WebSockets
==========

ws://host:port

Protocol | Description
--|--
web-console | Forwards input and output to/from the `servmc` application
