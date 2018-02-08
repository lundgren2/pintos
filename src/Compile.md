cd /home/osckl172/TDIU16/pintos/src && make clean &&
cd /home/osckl172/TDIU16/pintos/src/threads && make -j8 &&
cd /home/osckl172/TDIU16/pintos/src/userprog && make -j8 &&
cd /home/osckl172/TDIU16/pintos/src/examples && make -j8 &&
cd ../userprog

cd /home/osckl172/TDIU16/pintos/src/vm && make -j8 &&
cd /home/osckl172/TDIU16/pintos/src/filesys && make -j8 && 


TO README: 
## TODO: 
Q: Sista punkten sid 72: https://www.ida.liu.se/~TDIU16/2017/lab/pdf/17sysexec.pdf
A: Nej, vi hade glömt att implementera process_cleanup i labben, lol.

* Det verkar som att det är fel i stacken. Vi exekviverar två program åt gången i lab 17 longrun. Säkert något gammalt fel som spökar.
Tips: Kolla rad 325, 
    // HACK if_.esp -= 12; /* Unacceptable solution. */
    if_.esp = setup_main_stack(parameters->command_line, if_.esp);


## Labbar: 
v14: 8, 11
v15: 10

