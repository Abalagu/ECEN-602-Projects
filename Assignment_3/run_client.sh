# client shell

clear

file_name=Makefile

# create test folder, included in .gitignore
test_folder=./test
if [ ! -d "$test_folder" ]; then
    mkdir $test_folder
fi

client_folder=$test_folder/tftp_client
server_folder=$test_folder/tftp_server

# check existence, then move to testing folder
if [ -d "$client_folder" ]; then
    cd $client_folder
else
    mkdir $client_folder
    cd $client_folder
fi

# test command from tftp
tftp localhost 4950 -v -m binary -c get $file_name

# compare files
cmp $client_folder/$file_name $server_folder/$file_name
retval=$?
if [ $retval -eq 0 ]; then
    echo same file
fi
