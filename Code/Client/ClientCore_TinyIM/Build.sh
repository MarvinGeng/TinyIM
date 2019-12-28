#!/bin/bash
Build()
{
	mkdir Linux
	cd Linux
	cmake ..
	make 
	cd ..
        rm -rf Linux
}

Clean()
{
   rm -rf Linux
   rm -rf bin
}
if [ $1 = "clean" ] ; then
  Clean
 echo 'Clean'
elif [ $1 = "build" ] ;then
	Build
fi


