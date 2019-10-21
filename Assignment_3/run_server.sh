# server shell

clear
 # change this file name
file_name=sample.txt

# create test folder, included in .gitignore
test_folder=./test
if [ ! -d "$test_folder" ]; then
    mkdir $test_folder
fi

server_folder=$test_folder/tftp_server
# check server folder existence
if [ ! -d "$server_folder" ]; then
    mkdir $server_folder
fi

test_file=$server_folder/$file_name
# check test file existence
if [ ! -f "$test_file" ]; then
    openssl rand -out $test_file -base64 $((2 ** 30 * 3 / 4))
    echo "test file $test_file generated."
else
    echo "test file $test_file exists."
fi

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
