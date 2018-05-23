#!/bin/bash

TIMES_SUCCEEDED=0
for i in {1..10}
do
make check -j8
if [ $? -eq 0 ];then
    TIMES_SUCCEEDED=$((${TIMES_SUCCEEDED} + 1))
fi
rm -rf build/tests/userprog/*.output
done

echo "Ran test suite ${TIMES_SUCCEEDED} successful times"
