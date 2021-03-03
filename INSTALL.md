INSTALLATION
============

Build dependencies
------------------
OpenLDAP sources must be available. For an easier build, copy all ppm module
into contrib/slapd-modules OpenLDAP source directory.

Build
-----
Be sure to have copied ppm module into contrib/slapd-modules OpenLDAP source
directory.

Adapt the Makefile command to indicate:
LDAP_SRC: path to OpenLDAP source directory
LDAP_BUILD: (optionally) path to OpenLDAP object files (same as above if undefined)
CONFIG: where the ppm.example file will finally stand
        note: ppm configuration now lies into pwdCheckModuleArg password policy attribute
              provided example file is only helpful as an example or for testing
LIBDIR: where the library will be installed
DEBUG: If defined, ppm logs its actions with syslog

If necessary, you can also adapt some OpenLDAP source directories (if changed):
LDAP_INC : OpenLDAP headers directory
LDAP_LIBS : OpenLDAP built libraries directory


Here is an illustrative example showing how to use the options:
(based upon the default prefix /usr/local of OpenLDAP)

```
make clean
make CONFIG=/usr/local/openldap/etc/ppm.example LDAP_SRC=../../..
make test LDAP_SRC=../../..
make install CONFIG=/usr/local/openldap/etc/ppm.example LIBDIR=/usr/local/lib
```

