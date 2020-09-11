#!/usr/bin/env bash

#to allow double click execution go to cmd & execute:
#gsettings set org.gnome.nautilus.preferences executable-text-activation 'launch'
#and open the file properties->permissions & mark as executable

#https://askubuntu.com/questions/138908/how-to-execute-a-script-just-by-double-clicking-like-exe-files-in-windows


echo "Compile? [Y/N]"
read string # Ask the user to enter a string
if [[ $string == "y" ]] || [[ $string == "Y" ]]
then
	make clean
	make output
fi

echo "Execute? [Y/N]"
read string # Ask the user to enter a string
if [[ $string == "y" ]] || [[ $string == "Y" ]]
then
	./output
	exit 1
else
	exit 1
fi