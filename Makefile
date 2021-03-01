# contrib/slapd-modules/ppm/Makefile
# Copyright 2014 David Coutadeur, Paris. All Rights Reserved.
#

CC=gcc

# Path to the OpenLDAP source tree
LDAP_SRC=../../..

# Path to the OpenLDAP object tree - same as above unless
# you're doing out-of-tree builds.
LDAP_BUILD=$(LDAP_SRC)


# Where the ppm example file should be installed
CONFIG=/etc/openldap/ppm.example
# Path of OpenLDAP installed libs, where the ppm library should be installed
LIBDIR=/usr/lib/openldap

OPT=-g -O2 -Wall -fpic 						\
	-DCONFIG_FILE="\"$(CONFIG)\""				\
	-DDEBUG

# Where to find the OpenLDAP headers.

LDAP_INC=-I$(LDAP_SRC)/include \
	 -I$(LDAP_SRC)/servers/slapd

# Where to find the OpenLDAP libraries.

LDAP_LIBS=-L$(LDAP_BUILD)/libraries/liblber/.libs \
	  -L$(LDAP_BUILD)/libraries/libldap/.libs

CRACK_INC=-DCRACKLIB

INCS=$(LDAP_INC) $(CRACK_INC)

LDAP_LIB=-lldap -llber

CRACK_LIB=-lcrack

LIBS=$(LDAP_LIB) $(CRACK_LIB)

TESTS=./unit_tests.sh



all: 	ppm ppm_test

ppm_test: 
	$(CC) -g $(LDAP_INC) $(LDAP_LIBS) -Wl,-rpath=. -o ppm_test ppm_test.c ppm.so $(LIBS)

ppm.o:
	$(CC) $(OPT) -c $(INCS) ppm.c

ppm: ppm.o
	$(CC) $(LDAP_INC) -shared -o ppm.so ppm.o $(CRACK_LIB)

install: ppm
	cp -f ppm.so $(LIBDIR)
	cp -f ppm_test $(LIBDIR)
	cp -f ppm.example $(CONFIG)

.PHONY: clean

clean:
	$(RM) -f ppm.o ppm.so ppm.lo ppm_test
	$(RM) -rf .libs

test: ppm ppm_test
	LDAP_SRC=$(LDAP_SRC) $(TESTS)


