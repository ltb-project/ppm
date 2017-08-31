# contrib/slapd-modules/ppm/Makefile
# Copyright 2014 David Coutadeur, Paris. All Rights Reserved.
#

CC=gcc

# Path to the configuration file
#
CONFIG=/etc/openldap/ppm.conf
LIBDIR=/usr/lib/openldap

OPT=-g -O2 -Wall -fpic 						\
	-DCONFIG_FILE="\"$(CONFIG)\""				\
	-DDEBUG

# Where to find the OpenLDAP headers.

LDAP_INC=-I../include \
	 -I../servers/slapd

# Where to find the OpenLDAP libraries.

LDAP_LIBS=-L../libraries/liblber/.libs \
	  -L../libraries/libldap_r/.libs

CRACK_INC=-DCRACKLIB

INCS=$(LDAP_INC) $(CRACK_INC)

LDAP_LIB=-lldap_r -llber

CRACK_LIB=-lcrack

LIBS=$(LDAP_LIB) $(CRACK_LIB)

TESTS=./unit_tests.sh



all: 	ppm ppm_test

ppm_test: 
	$(CC) $(LDAP_INC) $(LDAP_LIBS) -Wl,-rpath=. -o ppm_test ppm_test.c ppm.so $(LIBS)

ppm.o:
	$(CC) $(OPT) -c $(INCS) ppm.c

ppm: ppm.o
	$(CC) $(LDAP_INC) -shared -o ppm.so ppm.o $(CRACK_LIB)

install: ppm
	cp -f ppm.so $(LIBDIR)
	cp -f ppm_test $(LIBDIR)
	cp -f ppm.conf $(CONFIG)

clean:
	$(RM) ppm.o ppm.so ppm.lo ppm_test
	$(RM) -r .libs

test: ppm ppm_test
	$(TESTS)


