#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "./redismodule.h"

int RMUtil_StringEquals(RedisModuleString *s1, RedisModuleString *s2) {

  const char *c1, *c2;
  size_t l1, l2;
  c1 = RedisModule_StringPtrLen(s1, &l1);
  c2 = RedisModule_StringPtrLen(s2, &l2);
  if (l1 != l2) return 0;

  return strncmp(c1, c2, l1) == 0;
}

int RMUtil_ArgIndex(const char *arg, RedisModuleString **argv, int argc) {

  size_t larg = strlen(arg);
  for (int offset = 0; offset < argc; offset++) {
    size_t l;
    const char *carg = RedisModule_StringPtrLen(argv[offset], &l);
    if (l != larg) continue;
    if (carg != NULL && strncasecmp(carg, arg, larg) == 0) {
      return offset;
    }
  }
  return -1;
}

int rmutil_vparseArgs(RedisModuleString **argv, int argc, int offset, const char *    fmt, va_list ap) {
  
  int i = offset;
  char *c = (char *)fmt;
  while (*c && i < argc) {

    // read c string
    if (*c == 'c') {                                  
      char **p = va_arg(ap, char **);
      *p = (char *)RedisModule_StringPtrLen(argv[i], NULL);
    } else if (*c == 'b') { 
      char **p = va_arg(ap, char **);
      size_t *len = va_arg(ap, size_t *);
      *p = (char *)RedisModule_StringPtrLen(argv[i], len);
    } else if (*c == 's') {  // read redis string
    
      RedisModuleString **s = va_arg(ap, void *);
      *s = argv[i];

    } else if (*c == 'l') {  // read long
      long long *l = va_arg(ap, long long *);

      if (RedisModule_StringToLongLong(argv[i], l) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
      }
    } else if (*c == 'd') {  // read double
      double *d = va_arg(ap, double *);
      if (RedisModule_StringToDouble(argv[i], d) != REDISMODULE_OK) {
        return REDISMODULE_ERR;
      }
    } else if (*c == '*') {  // skip current arg
      // do nothing
    } else {
      return REDISMODULE_ERR;  // WAT?
    }
    c++;
    i++;
  }
  // if the format is longer than argc, retun an error
  if (*c != 0) {
    return REDISMODULE_ERR;
  }
  return REDISMODULE_OK;
}


int RMUtil_ParseArgsAfter(const char *token, RedisModuleString **argv, int argc, const char *fmt, ...) {

  int pos = RMUtil_ArgIndex(token, argv, argc);
  if (pos < 0) {
    return REDISMODULE_ERR;
  }

  va_list ap;
  va_start(ap, fmt);
  int rc = rmutil_vparseArgs(argv, argc, pos + 1, fmt, ap);
  va_end(ap);
  return rc;
}



