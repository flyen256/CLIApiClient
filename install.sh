#!/bin/bash

if ! which git > /dev/null 2>&1; then
	echo "Git is not installed";
	exit 1
fi

git clone https://github.com/flyen256/CLIApiClient.git
cd CLIApiClient
cmake -B build
cmake --build build
sudo cp ./build/CLIApiClient /usr/bin/CLIApiClient
