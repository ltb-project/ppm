/*
 * ppm.c for OpenLDAP
 *
 * See LICENSE, README and INSTALL files
 */

#include <stdlib.h>             // for type conversion, such as atoi...
#include <string.h>
#include <ctype.h>
#include <portable.h>
#include <slap.h>

#if defined(DEBUG)
#include <syslog.h>
#endif

#ifndef CONFIG_FILE
#define CONFIG_FILE                       "/etc/openldap/ppm.conf"
#endif

#define DEFAULT_QUALITY                   3
#define MEMORY_MARGIN                     50
#define MEM_INIT_SZ                       64
#define FILENAME_MAXLEN                   512

#define STATICCONF_MAX_SIZE               2
#define CLASS_MAX_SIZE                    10
#define PARAM_MAXLEN                      16
#define VALUE_MAXLEN                      60

#define PARAM_PREFIX_MIN_CRITERIA         "min-"
#define PARAM_PREFIX_MINFORPOINT_CRITERIA "minForPoint-"


#define PASSWORD_TOO_SHORT_SZ \
  "Password for dn=\"%s\" is too short (%d/6)"
#define PASSWORD_QUALITY_SZ \
  "Password for dn=\"%s\" does not pass required number of strength checks (%d of %d)"
#define PASSWORD_CRITERIA \
  "Password for dn=\"%s\" has not reached the minimum number of characters (%d) for class %s"
#define PASSWORD_FORBIDDENCHARS \
  "Password for dn=\"%s\" contains %d forbidden characters in %s"
#define BAD_PASSWORD_SZ \
  "Bad password for dn=\"%s\" because %s"


typedef struct conf {
    char param[PARAM_MAXLEN];
    enum {
        typeInt,
        typeStr
    } iType;
    union genValue {
        int iVal;
        char sVal[VALUE_MAXLEN];
    } value;
} conf;


int min(char *str1, char *str2);
void fillValue(conf * fillConf, char *value);
void fillParameter(char *word, char *classWord, char *value, conf * class,
                   int *toFill);
void load_config(char *word, char *value, conf * staticConf, conf * class,
                 int *min, int *minForPoint);
static void read_config_file(conf * staticConf, conf * class, int *min,
                             int *minForPoint);
int check_password(char *pPasswd, char **ppErrStr, Entry * pEntry);


void
strcpy_safe(char *dest, char *src, int length_dest)
{
    int length_src = strlen(src);
    int n = (length_dest < length_src) ? length_dest : length_src;
    // Copy the string — don’t copy too many bytes.
    strncpy(dest, src, n);
    // Ensure null-termination.
    dest[n] = '\0';
}

void
fillValue(conf * fillConf, char *value)
{
    switch ((*fillConf).iType) {
    case typeInt:
        (*fillConf).value.iVal = atoi(value);
#if defined(DEBUG)
        syslog(LOG_NOTICE, "ppm:  Accepted value: %d",
               (*fillConf).value.iVal);
#endif
        break;
    case typeStr:
        strcpy_safe((*fillConf).value.sVal, value, VALUE_MAXLEN);
#if defined(DEBUG)
        syslog(LOG_NOTICE, "ppm:  Accepted value: %s",
               (*fillConf).value.sVal);
#endif
        break;
    }
}

/* Function that save a parameter in the corresponding conf table
 * input: word: the parameter label to save
 *        classWord: the corresponding parameter name in the class table (without the min- or minForPoint- prefix)
 *        value: the value to save
 *        class: the class table
 *        toFill: the parameter table to fill. Can be min, minForPoint, or NULL
 *                if NULL, then means class is to be filled
 * output: nothing (the toFill conf table is completed with the new value, and
 *         the class parameter can be updated accordingly)
 *         If there is no place left in the *toFill table, then nothing is done !
 * */
void
fillParameter(char *word, char *classWord, char *value, conf * class,
                   int *toFill)
{
    int classFound = 0;
    int i = 0, indexEmpty = CLASS_MAX_SIZE;

    while ((i < CLASS_MAX_SIZE) && (classFound == 0)) {
        if ((strlen(classWord) == strlen(class[i].param))
            && (strncmp(classWord, class[i].param, strlen(class[i].param)) ==
                0)) {
            if (toFill == NULL) {
                fillValue(&class[i], value);
            } else {
                toFill[i] = atoi(value);
#if defined(DEBUG)
                syslog(LOG_NOTICE, "ppm:  Accepted value: %d",
                       toFill[i]);
#endif
            }

            classFound = 1;
        }
        if ((strlen(class[i].param) == 0) && (indexEmpty == CLASS_MAX_SIZE)) {  //there is enough place to store data here
            indexEmpty = i;
        }
        i++;
    }

    if ((classFound == 0) && (indexEmpty < CLASS_MAX_SIZE)) {
        if (toFill == NULL) {
            fillValue(&class[indexEmpty], value);
        } else {
            toFill[indexEmpty] = atoi(value);
#if defined(DEBUG)
            syslog(LOG_NOTICE, "ppm:  Accepted value: %d",
                   toFill[indexEmpty]);
#endif
        }

        strcpy_safe(class[indexEmpty].param, classWord, PARAM_MAXLEN);
    }
#if defined(DEBUG)
    else if ((classFound == 0) && (indexEmpty == CLASS_MAX_SIZE))
        syslog(LOG_WARNING,
               "ppm: Not enough place to store new parameter: %s",
               word);
#endif


}


void
load_config(char *word, char *value, conf * staticConf, conf * class,
            int *min, int *minForPoint)
{
    int i = 0;
    int staticFound = 0;

    // First scan the static parameters
    staticFound = 0;
    for (i = 0; i < STATICCONF_MAX_SIZE; i++) {
        if ((strlen(word) == strlen(staticConf[i].param))
            && (strncmp(word, staticConf[i].param, strlen(staticConf[i].param))
                == 0)) {
            fillValue(&staticConf[i], value);
#if defined(DEBUG)
            syslog(LOG_NOTICE, "ppm:  Accepted static parameter: %s",
                   word);
#endif
            staticFound = 1;
            break;
        }
    }


    if (staticFound != 1) {

        // Is this a min parameter ?
        if (strncmp
            (word, PARAM_PREFIX_MIN_CRITERIA,
             (sizeof(PARAM_PREFIX_MIN_CRITERIA) - 1)) == 0) {
#if defined(DEBUG)
            syslog(LOG_NOTICE, "ppm:  Accepted min parameter: %s",
                   word);
#endif
            fillParameter(word,
                          (word + (sizeof(PARAM_PREFIX_MIN_CRITERIA) - 1)),
                          value, class, min);
        }
        // Is this a minForPoint parameter ?
        else if (strncmp
                 (word, PARAM_PREFIX_MINFORPOINT_CRITERIA,
                  (sizeof(PARAM_PREFIX_MINFORPOINT_CRITERIA) - 1)) == 0) {
#if defined(DEBUG)
            syslog(LOG_NOTICE,
                   "ppm:  Accepted minForPoint parameter: %s", word);
#endif
            fillParameter(word,
                          (word +
                           (sizeof(PARAM_PREFIX_MINFORPOINT_CRITERIA) - 1)),
                          value, class, minForPoint);
        }

        else {
            // ok, thus if it is none of the previous, assume it's a class parameter
#if defined(DEBUG)
            syslog(LOG_NOTICE, "ppm:  Accepted class parameter: %s",
                   word);
#endif
            fillParameter(word, word, value, class, NULL);
        }

    }

}



static void
read_config_file(conf * staticConf, conf * class, int *min,
                             int *minForPoint)
{
    FILE *config;
    char line[260] = "";

    if ((config = fopen(CONFIG_FILE, "r")) == NULL) {
#if defined(DEBUG)
        syslog(LOG_ERR, "ppm: Opening file %s failed", CONFIG_FILE);
#endif

    }

    while (fgets(line, 256, config) != NULL) {
        char *start = line;
        char *word, *value;

        while (isspace(*start) && isascii(*start))
            start++;

        if (!isascii(*start))
            continue;

        if ((word = strtok(start, " \t"))) {
            if ((value = strtok(NULL, " \t")) == NULL)
                continue;
            if (strchr(value, '\n') != NULL)
                strchr(value, '\n')[0] = '\0';

#if defined(DEBUG)
            syslog(LOG_NOTICE, "ppm: Word = %s, value = %s", word,
                   value);
#endif

            load_config(word, value, staticConf, class, min, minForPoint);



        }
    }

    fclose(config);
}

static int
realloc_error_message(char **target, int curlen, int nextlen)
{
    if (curlen < nextlen + MEMORY_MARGIN) {
#if defined(DEBUG)
        syslog(LOG_WARNING,
               "ppm: Reallocating szErrStr from %d to %d", curlen,
               nextlen + MEMORY_MARGIN);
#endif
        ber_memfree(*target);
        curlen = nextlen + MEMORY_MARGIN;
        *target = (char *) ber_memalloc(curlen);
    }

    return curlen;
}

int
check_password(char *pPasswd, char **ppErrStr, Entry * pEntry)
{

    char *szErrStr = (char *) ber_memalloc(MEM_INIT_SZ);
    int mem_len = MEM_INIT_SZ;

    int minQuality;
    char forbiddenChars[VALUE_MAXLEN];
    int nForbiddenChars = 0;
    int nQuality = 0;
    int nbInClass[CLASS_MAX_SIZE];
    int i;

    for (i = 0; i < CLASS_MAX_SIZE; i++)
        nbInClass[i] = 0;

    /* Set default values */
    conf staticConf[STATICCONF_MAX_SIZE] = {
        {"minQuality", typeInt, {.iVal = DEFAULT_QUALITY}
         }
        ,
        {"forbiddenChars", typeStr, {.sVal = ""}
         }
    };
    conf class[CLASS_MAX_SIZE] = {
        {"upperCase", typeStr, {.sVal = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"}
         }
        ,
        {"lowerCase", typeStr, {.sVal = "abcdefghijklmnopqrstuvwxyz"}
         }
        ,
        {"digit", typeStr, {.sVal = "0123456789"}
         }
        ,
        {"special", typeStr,
         {.sVal = "<>,?;.:/!§ù%*µ^¨$£²&é~\"#'{([-|è`_\\ç^à@)]°=}+"}
         }
    };
    for (i = 4; i < CLASS_MAX_SIZE; i++) {
        strcpy(class[i].param, "");
        class[i].iType = typeStr;
        strcpy(class[i].value.sVal, "");
    }

    // required number of characters to validate the password
    int min[CLASS_MAX_SIZE];
    for (i = 0; i < CLASS_MAX_SIZE; i++)
        min[i] = 1;

    // minimum number of characters for having the class point
    int minForPoint[CLASS_MAX_SIZE] = { 0 };
    for (i = 0; i < CLASS_MAX_SIZE; i++)
        minForPoint[i] = 0;

    /* Read config file */
    read_config_file(staticConf, class, min, minForPoint);

    minQuality = staticConf[0].value.iVal;
    strcpy_safe(forbiddenChars, staticConf[1].value.sVal, VALUE_MAXLEN);


    /*The password must have at least minQuality strength points with one
     * point granted if the password contains at least minForPoint characters for each class
     * It must contains at least min chars of each class
     * It must not contain any char in forbiddenChar */

    for (i = 0; i < strlen(pPasswd); i++) {

        int n;
        for (n = 0; n < CLASS_MAX_SIZE; n++) {
            if (strchr(class[n].value.sVal, pPasswd[i]) != NULL) {
                (nbInClass[n])++;
            }
        }
        if (strchr(forbiddenChars, pPasswd[i]) != NULL) {
            nForbiddenChars++;
        }
    }

    // Password checking done, now loocking for minForPoint criteria
    for (i = 0; i < CLASS_MAX_SIZE; i++) {
        if ((nbInClass[i] >= minForPoint[i])
            && strlen(class[i].value.sVal) != 0) {
            // 1 point granted
            nQuality++;
#if defined(DEBUG)
            syslog(LOG_NOTICE, "ppm: 1 point granted for class %s",
                   class[i].param);
#endif
        }
    }

    if (nQuality < minQuality) {
        mem_len = realloc_error_message(&szErrStr, mem_len,
                                        strlen(PASSWORD_QUALITY_SZ) +
                                        strlen(pEntry->e_name.bv_val) + 4);
        sprintf(szErrStr, PASSWORD_QUALITY_SZ, pEntry->e_name.bv_val,
                nQuality, minQuality);
        goto fail;
    }
    // Password checking done, now loocking for constraintClass criteria
    for (i = 0; i < CLASS_MAX_SIZE; i++) {
        if ((nbInClass[i] < min[i]) && strlen(class[i].value.sVal) != 0) {
            // constraint is not satisfied... goto fail
            mem_len = realloc_error_message(&szErrStr, mem_len,
                                            strlen(PASSWORD_CRITERIA) +
                                            strlen(pEntry->e_name.bv_val) + 2 +
                                            PARAM_MAXLEN);
            sprintf(szErrStr, PASSWORD_CRITERIA, pEntry->e_name.bv_val, min[i],
                    class[i].param);
            goto fail;
        }
    }

    // Password checking done, now loocking for forbiddenChars criteria
    if (nForbiddenChars > 0) {  // at least 1 forbidden char... goto fail
        mem_len = realloc_error_message(&szErrStr, mem_len,
                                        strlen(PASSWORD_FORBIDDENCHARS) +
                                        strlen(pEntry->e_name.bv_val) + 2 +
                                        VALUE_MAXLEN);
        sprintf(szErrStr, PASSWORD_FORBIDDENCHARS, pEntry->e_name.bv_val,
                nForbiddenChars, forbiddenChars);
        goto fail;
    }

    *ppErrStr = strdup("");
    ber_memfree(szErrStr);
    return (LDAP_SUCCESS);

  fail:
    *ppErrStr = strdup(szErrStr);
    ber_memfree(szErrStr);
    return (EXIT_FAILURE);

}
