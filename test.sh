echo ECEN 602 Team 4 Akhilesh Rawat and Luming Xu Echo Service

PORT=12346
echo Server on port $PORT.
gnome-terminal -- bash -c "./echos $PORT; exec bash"
gnome-terminal -- bash -c "./echo localhost $PORT; exec bash"
#gnome-terminal -- bash -c "printf 'yyy\n' | ../binary/client localhost $PORT; exec bash"
#gnome-terminal -- bash -c "yes hello | ../binary/client localhost $PORT; exec bash"
#gnome-terminal -- bash -c "yes world | ../binary/client localhost $PORT; exec bash"
