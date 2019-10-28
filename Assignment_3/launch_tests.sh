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
echo -e "TEST CASE 1"
file_name=binary_2048B
tftp localhost 4950 -v -m binary -c get $file_name
# compare files
cmp $client_folder/$file_name $server_folder/$file_name
retval=$?
if [ $retval -eq 0 ]; then
    echo -e "test case 1: $file_name. diff: same file\n"
fi

# test case 2
echo -e "TEST CASE 2"
file_name=binary_2047B
tftp localhost 4950 -v -m binary -c get $file_name
# compare files
cmp $client_folder/$file_name $server_folder/$file_name
retval=$?
if [ $retval -eq 0 ]; then
    echo -e "test case 2: $file_name. diff: same file\n"
fi

# test case 3
echo -e "TEST CASE 3"
file_name=test_case_3.txt
tftp localhost 4950 -v -m ascii -c get $file_name
# compare files
cmp $client_folder/$file_name $server_folder/$file_name
retval=$?
if [ $retval -eq 0 ]; then
    echo -e "test case 2: $file_name. diff: same file\n"
fi

# test case 4
echo -e "TEST CASE 4"
file_name=binary_34MB
tftp localhost 4950 -v -m binary -c get $file_name
# compare files
cmp $client_folder/$file_name $server_folder/$file_name
retval=$?
if [ $retval -eq 0 ]; then
    echo "test case 4: $file_name. diff: same file"
    echo ""
fi

# test case 5
echo -e "TEST CASE 5"
file_name=non_existent
tftp localhost 4950 -v -m binary -c get $file_name

# test case 6
echo -e "\nTEST CASE 6: see local screenshot\n"

# test case 7
echo -e "TEST CASE 7"
file_name=binary_34MB
# test the behavior of server when client disconnects in the middle of transmission
echo "!Press Ctrl+C for test case 7"
tftp localhost 4950 -v -m binary -c get $file_name
echo "should see timeout on server termiinal"

# rename local client files to different names, to avoid filename collision

# test case 8.1
echo -e "\nTEST CASE 8.1 put 2048B"
mv binary_2048B binary_2048B.bak
file_name=binary_2048B.bak
tftp localhost 4950 -v -m binary -c put $file_name
cmp $client_folder/$file_name $server_folder/$file_name
retval=$?
if [ $retval -eq 0 ]; then
    echo -e "test case 2: $file_name. diff: same file\n"
fi


# test case 8.2
echo -e "\nTEST CASE 8.2 put 2047B"
mv binary_2047B binary_2047B.bak
file_name=binary_2047B.bak
tftp localhost 4950 -v -m binary -c put $file_name
cmp $client_folder/$file_name $server_folder/$file_name
retval=$?
if [ $retval -eq 0 ]; then
    echo -e "test case 2: $file_name. diff: same file\n"
fi

# test case 8.3
echo -e "\nTEST CASE 8.3 put 34MB"
mv binary_34MB binary_34MB.bak
file_name=binary_34MB.bak
tftp localhost 4950 -v -m binary -c put $file_name
cmp $client_folder/$file_name $server_folder/$file_name
retval=$?
if [ $retval -eq 0 ]; then
    echo -e "test case 2: $file_name. diff: same file\n"
fi

# test case 8.4
echo -e "\nTEST CASE 8.4 put ascii file"
mv test_case_3.txt test_case_3.txt.bak
file_name=test_case_3.txt.bak
tftp localhost 4950 -v -m binary -c put $file_name
cmp $client_folder/$file_name $server_folder/$file_name
retval=$?
if [ $retval -eq 0 ]; then
    echo -e "test case 2: $file_name. diff: same file\n"
fi

# test case 8.5
echo -e "\nTEST CASE 8.5 non-existent file"
tftp localhost 4950 -v -m binary -c put whatever_nonexistent

# test case 8.6
echo -e "TEST CASE 8.6: doesn't need simutaneous connection support from server to client"

# test case 8.7
echo -e "\nTEST CASE 8.7 put 34MB\n !Press Ctrl+C to terminate"
mv binary_34MB.bak binary_34MB.bak.bak
file_name=binary_34MB.bak.bak
tftp localhost 4950 -v -m binary -c put $file_name
