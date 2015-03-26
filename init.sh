#!/bin/bash
sudo su -


# get redis and deps
apt-get -y -f install git redis-server clang libcurl4-openssl-dev build-essential libtidy-dev

# hiredis
cd /opt
git clone https://github.com/redis/hiredis.git
cd hiredis
make
make install


# crawler
cd ~
git clone https://github.com/prosbloom225/cRawler.git
cd cRawler
make

# logging
mkdir /var/log/crawler
chmod 757 /var/log/crawler
