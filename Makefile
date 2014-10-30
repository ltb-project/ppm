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
#
LDAP_INC=-I../../../include \
	 -I../../../servers/slapd

INCS=$(LDAP_INC) $(CRACK_INC)

LDAP_LIB=-lldap_r -llber

LIBS=$(LDAP_LIB)



all: 	ppm

ppm.o:
	$(CC) $(OPT) -c $(INCS) ppm.c

ppm: clean ppm.o
	$(CC) $(LDAP_INC) -shared -o ppm.so ppm.o

install: ppm
	cp -f ppm.so $(LIBDIR)
	cp -f ppm.conf $(CONFIG)

clean:
	$(RM) ppm.o ppm.so ppm.lo
	$(RM) -r .libs

