# echo "Running test-zem.c"
# g++ test-zem.c zemaphore.c -o test-zem -lpthread
# ./test-zem
# echo
# echo "Running test-toggle.c"
# g++ test-toggle.c zemaphore.c -o test-toggle -lpthread
# ./test-toggle

echo "Running 1805115.c"
g++ 1805115.c zemaphore.c -o 1805115 -lpthread
./1805115
