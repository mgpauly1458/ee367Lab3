./server &
sleep 0.5
./client wiliki.eng.hawaii.edu
sleep 0.5
pkill server
