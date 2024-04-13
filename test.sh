#!/bin/bash

TEST_RES=$(tail -16 test.out | grep true | wc -l)

if [ $TEST_RES = 16 ];
then
  echo "Test passed"
else
  echo "Test failed"
fi	
