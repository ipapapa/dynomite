#!/bin/bash
# Script launched by Florida.
#

# Start redis-server if not running
if ! (ps -ef | grep -q '[/]apps/nfredis/bin/redis-server'); then
    (/apps/nfredis/bin/launch_nfredis.sh &)
fi

# ** This check has to be after the redis check above.
# Quit if Dynomite is already running, rather than
# throwing errors from this script.
if (ps -ef | grep  '[/]apps/dynomite/bin/dynomite'); then
    logger -s "Dynomite already running, no need to restart it again."
    exit 0
fi

DYN_DIR=/apps/dynomite
LOG_DIR=/logs/dynomite
CONF_DIR=$DYN_DIR/conf

source /etc/profile.d/netflix_environment.sh

declare -x `stat --printf "userowner=%U\ngroupowner=%G\n" "$0"`

if [ ! -d "$LOG_DIR" ]; then
    sudo mkdir -p $LOG_DIR
    sudo chown -R $userowner:$groupowner $LOG_DIR
fi

#save the previous log
if [ -e $LOG_DIR/dynomite.log ]; then
    mv $LOG_DIR/dynomite.log $LOG_DIR/dynomite-$(date +%Y%m%d_%H%M%S).log
fi

# note that we do not use 'su - username .... ' , because we want to keep the env settings that we have done so far
cmd="$DYN_DIR/bin/dynomite -d -c $CONF_DIR/dynomite.yml --output=$LOG_DIR/dynomite.log && sudo /apps/dynomite/bin/core_affinity.sh"

echo "Running command: $cmd"

sudo sh -c "ulimit -c unlimited &&  exec sudo /usr/local/bin/setuidgid $NETFLIX_APPUSER $cmd"

