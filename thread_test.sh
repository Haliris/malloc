#!/bin/bash

log_file="thread_test_dump.log"

echo_color() {
    local text="$1"
    local color_code="$2"
    echo -e "${color_code}${text}\e[0m"  # \e[0m resets color attributes
}

touch $log_file

for i in {1..10}
do
    ./malloc_test > $log_file
    if [ $? -eq 0 ]
    then
        continue
    else
        echo_color "Test failed: " "\e[31m"
        cat $log_file
    fi
done
echo "Test done running"
cat $log_file
rm -rf $log_file
