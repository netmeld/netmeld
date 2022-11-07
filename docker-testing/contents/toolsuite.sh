#!/bin/bash


sudo service postgresql start

./datalake-datastore.sh
./fetchers.sh
./clw.sh