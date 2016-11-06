#!/bin/bash
echo Running parse_test_positve
echo ----------------------------------------------------------
./compiler467 -Tp ./cases/parse_test_positve.frag

echo ----------------------------------------------------------
echo Running parse_test_negative
echo ----------------------------------------------------------
./compiler467 -Tp ./cases/parse_test_negative.frag 
echo ----------------------------------------------------------

echo done
