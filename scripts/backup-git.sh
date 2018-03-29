#!/bin/bash -e
dir="$1"
ver="$2"
world="$(sed -ne 's/^level-name=\(.*\)$/\1/p' "$dir/server.properties")"

cd "$dir/$world"
if [ ! -e .git ]; then
	name="$(grep $USER /etc/passwd | cut -d: -f5 | cut -d, -f1)"
	[ -z "$name" ] && name="$(whoami)"
	git init
	git config user.name "$name"
	git config user.email "$(whoami)@$(hostname)"
	git config commit.gpgSign false
	git config core.compression 9
fi

time="$(date +%Y-%m-%dT%H:%M:%S%z)"
msg="$world $time $ver"
echo "Backup as: $msg"

git add .
git commit -m "$msg"
git gc
