# server shell

clear

BASEDIR=$PWD

# create test folder, included in .gitignore
test_folder=$BASEDIR/test
if [ ! -d "$test_folder" ]; then
    mkdir $test_folder
fi

server_folder=$test_folder/server
# check server folder existence
if [ ! -d "$server_folder" ]; then
    mkdir $server_folder
fi

# test case 1: 2048 Bytes
base64 /dev/urandom | head -c 2K > $server_folder/binary_2048B
# test case 2: 2047 Bytes
base64 /dev/urandom | head -c 2047 > $server_folder/binary_2047B
# test case 3: text file with several CRs, copy from main folder
cp $BASEDIR/test_case_3.txt $server_folder/test_case_3.txt
# test case 4: 34 MB
base64 /dev/urandom | head -c 34MB > $server_folder/binary_34MB
echo "test files generated."

make clean
make
retval=$? # store retval of make

if [ $retval -eq 0 ]; then # make without error
    mv ./server $server_folder # move server to test directory
    cd $server_folder
    pwd
    ./server 4950 # launch server
else
    echo MAKE ERROR.
fi
