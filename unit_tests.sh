#!/bin/bash

# Launch unitary tests
# 


CONFIG_FILE="ppm.conf"
LIB_PATH="."

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

RESULT=0

PPM_CONF_1='minQuality 3
maxLength 0
checkRDN 0
forbiddenChars 
maxConsecutivePerClass 0
class-upperCase ABCDEFGHIJKLMNOPQRSTUVWXYZ 0 1
class-lowerCase abcdefghijklmnopqrstuvwxyz 0 1
class-digit 0123456789 0 1
class-special <>,?;.:/!§ù%*µ^¨$£²&é~"#'\''{([-|è`_\ç^à@)]°=}+ 0 1'

PPM_CONF_2='minQuality 3
maxLength 20
checkRDN 0
forbiddenChars à
maxConsecutivePerClass 5
class-upperCase ABCDEFGHIJKLMNOPQRSTUVWXYZ 2 4
class-lowerCase abcdefghijklmnopqrstuvwxyz 3 4
class-digit 0123456789 2 4
class-special <>,?;.:/!§ù%*µ^¨$£²&é~"#'\''{([-|è`_\ç^à@)]°=}+ 0 4'

echo "$PPM_CONF_1" > ppm1.conf
echo "$PPM_CONF_2" > ppm2.conf


launch_test()
{
  # launch tests
  # FORMAT: launch_test [conf_file] [password] [expected_result]
  # [expected_result] = [PASS|FAIL]

  local CONF="$1"
  local PASS="$2"
  local EXPECT="$3"

  [[ $EXPECT == "PASS" ]] && EXP="0" || EXP="1"

  PPM_CONFIG_FILE="${CONF}" LD_LIBRARY_PATH="${LIB_PATH}" ./ppm_test "${PASS}"
  RES="$?"

  if [ "$RES" -eq "$EXP" ] ; then
    echo -e "conf=${CONF} pass=${PASS} expect=${EXPECT}... ${GREEN}PASS${NC}"
  else
    echo -e "conf=${CONF} pass=${PASS} expect=${EXPECT}... ${RED}FAIL${NC}"
    ((RESULT+=1))
  fi

  echo
}




launch_test "ppm1.conf" "azerty" "FAIL"
launch_test "ppm1.conf" "azeRTY" "FAIL"
launch_test "ppm1.conf" "azeRTY123" "PASS"
launch_test "ppm1.conf" "azeRTY." "PASS"


launch_test "ppm2.conf" "AAaaa01AAaaa01AAaaa0" "PASS"
# too long
launch_test "ppm2.conf" "AAaaa01AAaaa01AAaaa01" "FAIL"
# forbidden char
launch_test "ppm2.conf" "AAaaa01AAaaa01AAaaaà" "FAIL"
# too much consecutive for upper
launch_test "ppm2.conf" "AAaaa01AAaaa01AAAAAA" "FAIL"
# not enough upper
launch_test "ppm2.conf" "Aaaaa01aaaaa01aa.;.;" "FAIL"
# not enough lower
launch_test "ppm2.conf" "aaAAA01BB0123AAA.;.;" "FAIL"
# not enough digit
launch_test "ppm2.conf" "1AAAA.;BBB.;.;AA.;.;" "FAIL"
# not enough points (no point for digit)
launch_test "ppm2.conf" "AAaaaBBBBaaa01AAaaaa" "FAIL"



echo "${RESULT} error(s) encountered"

rm ppm1.conf ppm2.conf
exit ${RESULT}

