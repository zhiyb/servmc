#!/bin/bash -e
dir="$1"
ver="$2"
world="$(sed -ne 's/^level-name=\(.*\)$/\1/p' "$dir/server.properties")"

cd "$dir/$world"
if [ ! -e .git ]; then
	git init
	git config user.name "$(grep $USER /etc/passwd | cut -d: -f5 | cut -d, -f1)"
	git config user.email "$(whoami)@$(hostname)"
	git config commit.gpgSign false
	git config core.compression 9
fi

time="$(date -Iseconds)"
msg="$world $time $ver"
echo "Backup as: $msg"

git add .
git commit -m "$msg"
