#!/bin/bash
Build()
{
cd ./Server/ChatServer
./Build.sh build
cd ../..
cd Client/ClientCore_TinyIM
./Build.sh build
cd ../..
}

Clean()
{
cd ./Server/ChatServer
./Build.sh clean
cd ../..
cd Client/ClientCore_TinyIM
./Build.sh Clean
}

if [ $1 = "clean" ] ; then
   Clean
   echo 'Clean'
elif [ $1 = "build" ] ;then
   Build
fi

