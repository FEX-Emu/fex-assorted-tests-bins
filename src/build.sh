gcc smc.cpp -g -o ../smc -DEXECSTACK -z execstack
gcc smc.cpp -g -o ../smc-static -DEXECSTACK -DOMAGIC -static -Wl,--omagic -z execstack # to test the elf loader
gcc smc-mt.cpp -g -o ../smc-mt -lpthread
gcc smc-mt-2.cpp -g -o ../smc-mt-2 -lpthread

