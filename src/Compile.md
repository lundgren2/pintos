cd /home/osckl172/TDIU16/pintos/src && make clean &&
cd /home/osckl172/TDIU16/pintos/src/threads && make -j8 &&
cd /home/osckl172/TDIU16/pintos/src/userprog && make -j8 &&
cd /home/osckl172/TDIU16/pintos/src/examples && make -j8 &&
cd ../userprog

cd /home/toblu933/TDIU16/pintos/src && make clean &&
cd /home/toblu933/TDIU16/pintos/src/threads && make -j8 &&
cd /home/toblu933/TDIU16/pintos/src/userprog && make -j8 &&
cd /home/toblu933/TDIU16/pintos/src/examples && make -j8 &&
cd ../userprog

TO README:

## TODO:

Q: Sista punkten sid 72: https://www.ida.liu.se/~TDIU16/2017/lab/pdf/17sysexec.pdf
A: Nej, vi hade glömt att implementera process_cleanup i labben, lol.

* Det verkar som att det är fel i stacken. Vi exekviverar två program åt gången i lab 17 longrun. Säkert något gammalt fel som spökar.
  Tips: Kolla rad 325,
  // HACK if*.esp -= 12; /* Unacceptable solution. */
  if*.esp = setup*main_stack(parameters->command_line, if*.esp);

## Labbar:

v14: 8, 11
v15: 10

## Tests

pintos -v -k --fs-disk=2 --qemu -p ../examples/longrun_nowait -a nowait -p ../examples/generic_parent -a generic_parent -p ../examples/busy -a dummy -- -f -q run 'nowait 10 50'
