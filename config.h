#pragma once

// Event polling tick interval
#define TICK_INTERVAL		10
// Backup schedule interval
#define BACKUP_INTERVAL		(1 * 60 * 60)
// Update checking interval
#define UPDATE_INTERVAL		(1 * 60 * 60)
// Restart interval
#define RESTART_INTERVAL	10

// Version configuration
// snapshot / release
#define UPDATE_TYPE	"snapshot"

// File paths and URLs
#define SERVER_PATH	"server"
#define WEB_PATH	"www"
#define MANIFEST_URL	"https://launchermeta.mojang.com/mc/game/version_manifest.json"

// Programs and arguments
#define EXEC_JAVA	"java"
#define EXEC_ARGS	"-jar", jar, "nogui"
#define EXEC_BACKUP	"scripts/backup.sh"
// Echo commands executed by management console
#define ECHO_CMD	1
// Web server port
#define WEB_PORT	25580
// Command line input prompt
#define INPUT_PROMPT	"servmc> "

// Server info string matching regression expressions
#define REGEX_SERVER(type) "^\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\] " \
			"\\[Server thread\\/" #type "\\]: "
#define REGEX_READY	REGEX_SERVER(INFO) "Done \\([0-9.]+s\\)!"
#define REGEX_LOGIN	REGEX_SERVER(INFO) "([^\\s]+) joined the game$"
#define REGEX_LOGOUT	REGEX_SERVER(INFO) "([^\\s]+) left the game$"
#define REGEX_PLAYERS	REGEX_SERVER(INFO) "There are ([0-9]+)( of a max |\\/)[0-9]+ players online:"
#define REGEX_SAVE	REGEX_SERVER(INFO) "Saved the (game|world)$"
#define REGEX_LINE	REGEX_SERVER(INFO)
// Server control commands
#define CMD_SAVE_ON	"save-on"
#define CMD_SAVE_OFF	"save-off"
#define CMD_SAVE_ALL	"save-all flush"
#define CMD_SAVE_DONE	"say §a"
#define CMD_SAVE_FAIL	"say §c"
#define CMD_PLAYERS	"list"
#define CMD_STOP	"stop"
#define CMD_STOP_MSG	"say §b"

// Coloured outputs
#define CLR_INPUT	"\33[97m"
#define CLR_STDOUT	"\33[0m"
#define CLR_STDERR	"\33[1;31m"
#define CLR_ECHO	"\33[90m"
#define CLR_MESSAGE	"\33[36m"
#define CLR_BACKUP	"\33[32m"
#define CLR_UPDATE	"\33[35m"
#define CLR_ERROR	"\33[31m"
#define CLR_EXTERNAL	"\33[33m"
#define CLR_LIBWS	"\33[34m"
#define CLR_WEB		"\33[94m"
