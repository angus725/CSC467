#!/bin/bash
echo Running parse_test_positve
echo ----------------------------------------------------------
./compiler467 -Tp ./cases/parse_test_positve.frag

echo ----------------------------------------------------------
echo Running parse_test_negative
echo ----------------------------------------------------------
./compiler467 -Tp ./cases/parse_test_negative.frag 
echo ----------------------------------------------------------
echo Running parse_test_negative2
echo ----------------------------------------------------------
./compiler467 -Tp ./cases/parse_test_negative2.frag 
echo ----------------------------------------------------------
echo Running parse_test_negative3
echo ----------------------------------------------------------
./compiler467 -Tp ./cases/parse_test_negative3.frag 
echo ----------------------------------------------------------
echo Running parse_test_negative4
echo ----------------------------------------------------------
./compiler467 -Tp ./cases/parse_test_negative4.frag 
echo ----------------------------------------------------------
echo Running parse_test_negative5
echo ----------------------------------------------------------
./compiler467 -Tp ./cases/parse_test_negative5.frag 
echo ----------------------------------------------------------
echo Running parse_test_negative6
echo ----------------------------------------------------------
./compiler467 -Tp ./cases/parse_test_negative6.frag 
echo ----------------------------------------------------------

echo done
