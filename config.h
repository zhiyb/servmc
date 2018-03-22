#pragma once

// Event polling tick interval
#define TICK_INTERVAL	10
// Backup schedule interval
#define BACKUP_INTERVAL	30
// Update checking interval
#define UPDATE_INTERVAL	10

// Version configuration
// snapshot / release
#define UPDATE_TYPE	"snapshot"

// File paths and URLs
#define SERVER_PATH	"server"
#define MANIFEST_URL	"https://launchermeta.mojang.com/mc/game/version_manifest.json"

// Programs and arguments
#define EXEC_JAVA	"java"
#define EXEC_ARGS	"-jar", jar, "nogui"

// Server info string matching regression expressions
#define REGEX_SERVER(type) "^\\[[0-9]{2}:[0-9]{2}:[0-9]{2}\\] " \
			"\\[Server thread\\/" #type "\\]: "
#define REGEX_READY	REGEX_SERVER(INFO) "Done \\([0-9.]+s\\)!"
#define REGEX_LOGIN	REGEX_SERVER(INFO) "[^\\s]+ joined the game$"
#define REGEX_PLAYERS	REGEX_SERVER(INFO) "There are ([0-9]+) of a max [0-9]+ players online:"
#define REGEX_SAVE	REGEX_SERVER(INFO) "Saved the game$"
#define REGEX_LINE	REGEX_SERVER(INFO)
// Server control commands
#define CMD_SAVE_ON	"save-on"
#define CMD_SAVE_OFF	"save-off"
#define CMD_SAVE_ALL	"save-all"
#define CMD_SAVE_MSG	"say Backup done."
#define CMD_PLAYERS	"list"
#define CMD_SHUTDOWN	"stop"
