
ppm.c - OpenLDAP password policy module

2014    David Coutadeur <david.coutadeur@gmail.com>

ppm.c is an OpenLDAP module for checking password quality when they are modified.
Passwords are checked against the presence or absence of certain character classes.

This module is used as an extension of the OpenLDAP password policy controls,
see slapo-ppolicy(5) section pwdCheckModule.



INSTALLATION
------------

See INSTALL file


USAGE
-----

See INSTALL file


Password checks
---------------

- 4 character classes are defined by default:
upper case, lower case, digits and special characters.

- more character classes can be defined, just write your own.

- passwords must match the amount of quality points.
A point is validated when at least m characters of the corresponding
character class are present in the password.

- passwords must have at least n of the corresponding character class
present, else they are rejected

- the two previous criterias are checked against any specific character class
defined.

- if a password contains any of the forbidden characters, then it is
rejected.


Configuration file
------------------

The configuration file (/etc/openldap/ppm.conf by default) contains
parameters for the module. If the file is not found, parameters are given their
default value.

The syntax of the file is :
parameter value

with spaces being delimiters. Parameter names ARE case sensitive

The default configuration file is the following:

minQuality 3
forbiddenChars
upperCase ABCDEFGHIJKLMNOPQRSTUVWXYZ
lowerCase abcdefghijklmnopqrstuvwxyz
digit 0123456789
special <>,?;.:/!§ù%*µ^¨$£²&é~"#'{([-|è`_\ç^à@)]°=}+
min-upperCase 0
min-lowerCase 0
min-digit 0
min-special 0
minForPoint-upperCase 1
minForPoint-lowerCase 1
minForPoint-digit 1
minForPoint-special 1


minQuality: integer
The password must have at least n different minFortPoint-* criterias validated.

forbiddentChars: string
If any of the characters present in this string are found in the password, then
the latter is rejected.

upperCase: string
lowerCase: string
digit: string
special: string
All 4 parameters are the definition of default character classes.
You can define your own character class, simply name it: myClass qwerty

min-[class]: integer
If the password does not contain at least n characters of the corresponding
class, then it is rejected.
You can define a min-[class] for your specific class. (default is 0)

minForPoint-[class]
If the password contains at least n characters of this character class, then
a point is granted. (used for minQuality)
You can define a minForPoint-[class] for your specific class. (default is 1)


Example
-------

With this policy:

minQuality 4
forbiddenChars .?,
upperCase ABCDEFGHIJKLMNOPQRSTUVWXYZ
lowerCase abcdefghijklmnopqrstuvwxyz
digit 0123456789
special <>,?;.:/!§ù%*µ^¨$£²&é~"#'{([-|è`_\ç^à@)]°=}+
myClass :)
min-upperCase 0
min-lowerCase 0
min-digit 0
min-special 0
min-myClass 1
minForPoint-upperCase 5
minForPoint-lowerCase 12
minForPoint-digit 1
minForPoint-special 1
minForPoint-myClass 1

the password:

ThereIsNoCowLevel)

is working, because,
- it has 4 character classes validated : upper, lower, special, and myClass
- it has no character among .?,
- it has at least one character among : or )


Logs
----
If a user password is rejected by ppm, the user will get this type of message:

Typical user message from ldappasswd(5):
  Result: Constraint violation (19)
  Additional info: Password for dn=\"%s\" does not pass required number of strength checks (2 of 3)

A more detailed message is written to the server log.

Server log:

ppm: Word = minQuality, value = 3
ppm:  Accepted value: 3
ppm:  Accepted static parameter: minQuality
ppm: Word = forbiddenChars, value =
ppm:  Accepted value:
ppm:  Accepted static parameter: forbiddenChars
ppm: Word = upperCase, value = ABCDEFGHIJKLMNOPQRSTUVWXYZ
ppm:  Accepted class parameter: upperCase
ppm:  Accepted value: ABCDEFGHIJKLMNOPQRSTUVWXYZ
ppm: Word = lowerCase, value = abcdefghijklmnopqrstuvwxyz
ppm:  Accepted class parameter: lowerCase
ppm:  Accepted value: abcdefghijklmnopqrstuvwxyz                                                         
ppm: Word = digit, value = 0123456789
ppm:  Accepted class parameter: digit
ppm:  Accepted value: 0123456789
ppm: Word = special, value = <>,?;.:/!Â§Ã¹%*Âµ^Â¨$Â£Â²&Ã©~"#'{([-|Ã¨`_\Ã§^Ã @)]Â°=}+
ppm:  Accepted class parameter: special
ppm:  Accepted value: <>,?;.:/!Â§Ã¹%*Âµ^Â¨$Â£Â²&Ã©~"#'{([-|Ã¨`_\Ã§^Ã @)]Â°=}+
ppm: Word = min-upperCase, value = 0
ppm:  Accepted min parameter: min-upperCase
ppm:  Accepted value: 0
ppm: Word = min-lowerCase, value = 0
ppm:  Accepted min parameter: min-lowerCase
ppm:  Accepted value: 0
ppm: Word = min-digit, value = 0
ppm:  Accepted min parameter: min-digit
ppm:  Accepted value: 0
ppm: Word = min-special, value = 0
ppm:  Accepted min parameter: min-special
ppm:  Accepted value: 0
ppm: Word = minForPoint-upperCase, value = 1
ppm:  Accepted minForPoint parameter: minForPoint-upperCase
ppm:  Accepted value: 1
ppm: Word = minForPoint-lowerCase, value = 1
ppm:  Accepted minForPoint parameter: minForPoint-lowerCase
ppm:  Accepted value: 1
ppm: Word = minForPoint-digit, value = 1
ppm:  Accepted minForPoint parameter: minForPoint-digit
ppm:  Accepted value: 1
ppm: Word = minForPoint-special, value = 1
ppm:  Accepted minForPoint parameter: minForPoint-special
ppm:  Accepted value: 1
ppm: 1 point granted for class upperCase
ppm: 1 point granted for class lowerCase


TODO
----
* integrate configuration file into cn=config


HISTORY
-------
* 2014-04-04 David Coutadeur <david.coutadeur@gmail.com>
  Version 1.0

