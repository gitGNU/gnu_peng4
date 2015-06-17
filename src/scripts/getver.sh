#! /bin/sh

## const char *peng_version = "4.01.00.0008"; /* CHANGEME */

grep peng_version.*CHANGEME peng.c |sed 's/.*"\([^"]*\)".*/\1/1'
