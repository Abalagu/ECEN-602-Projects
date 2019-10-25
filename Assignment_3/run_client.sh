# client shell

clear

BASEDIR=$PWD
# create test folder, included in .gitignore
test_folder=$BASEDIR/test
if [ ! -d "$test_folder" ]; then
    mkdir $test_folder
fi

client_folder=$test_folder/client
server_folder=$test_folder/server

# check existence, then move to testing folder
if [ -d "$client_folder" ]; then
    cd $client_folder
else
    mkdir $client_folder
    cd $client_folder
fi

# test case 1
file_name=binary_2048B
tftp localhost 4950 -v -m binary -c get $file_name
# compare files
cmp $client_folder/$file_name $server_folder/$file_name
retval=$?
if [ $retval -eq 0 ]; then
    echo "test case 1: $file_name. diff: same file"
    echo ""
fi

# test case 2
file_name=binary_2047B
tftp localhost 4950 -v -m binary -c get $file_name
# compare files
cmp $client_folder/$file_name $server_folder/$file_name
retval=$?
if [ $retval -eq 0 ]; then
    echo "test case 2: $file_name. diff: same file"
    echo ""
fi

# test case 3
file_name=test_case_3.txt
tftp localhost 4950 -v -m ascii -c get $file_name
# compare files
cmp $client_folder/$file_name $server_folder/$file_name
retval=$?
if [ $retval -eq 0 ]; then
    echo "test case 2: $file_name. diff: same file"
    echo ""
fi

# test case 4
file_name=binary_34MB
tftp localhost 4950 -v -m binary -c get $file_name
# compare files
cmp $client_folder/$file_name $server_folder/$file_name
retval=$?
if [ $retval -eq 0 ]; then
    echo "test case 4: $file_name. diff: same file"
    echo ""
fi