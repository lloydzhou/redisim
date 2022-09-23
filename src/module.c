#define REDISMODULE_EXPERIMENTAL_API
#include <time.h>
#include <math.h>
#include "./redismodule.h"
#include "./rmutil/util.h"
#include "./rmutil/strings.h"
#include "./rmutil/test_util.h"
#include "./rmutil/periodic.c"

#define C2S(ctx, s) RedisModule_CreateString(ctx, s, strlen(s))

int ListResponse(RedisModuleCtx *ctx, const char *fmt, ...) {
  va_list argv;
  int i = 0;
  // int argc = 0;
  RedisModuleString *s;
  char *c;
  long long l;
  unsigned u;
  va_start(argv, fmt);
  // RedisModule_Log(ctx, "warning", "fmt %s", fmt);
  RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
  while(*fmt) {
    // RedisModule_Log(ctx, "warning", "fmt %d %s", i, fmt);
    switch(*fmt) {
      case 's':
        s = va_arg(argv, RedisModuleString*);
        RedisModule_ReplyWithString(ctx, s);
        // RedisModule_Log(ctx, "warning", "argv %d %s", i, RedisModule_StringPtrLen(s, NULL));
        break;
      case 'c':
        c = va_arg(argv, char*);
        RedisModule_ReplyWithSimpleString(ctx, c);
        // RedisModule_Log(ctx, "warning", "argv %d %s", i, c);
        break;
      case 'l':
        l = va_arg(argv, long long);
        RedisModule_ReplyWithLongLong(ctx, l);
        // RedisModule_Log(ctx, "warning", "argv %d %lld", i, l);
        break;
      case 'u':
        u = va_arg(argv, unsigned);
        RedisModule_ReplyWithLongLong(ctx, u);
        // RedisModule_Log(ctx, "warning", "argv %d %u", i, u);
        break;
      default:
        break;
    }
    fmt++;
    i++;
  }
  RedisModule_ReplySetArrayLength(ctx, i);
  // RedisModule_Log(ctx, "warning", "argc %d", i);
  va_end(argv);
  return REDISMODULE_OK;
}

/**
 * IM.RECIVE  [<uid>] START [start]
 */

int ReciveCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  // RedisModule_Log(ctx, "warning", "argc %d", argc);
  if (argc < 2 || argc > 4) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModuleString *user_stream_key = RedisModule_CreateStringPrintf(ctx, "s:%s", RedisModule_StringPtrLen(argv[1], NULL));
  RedisModuleString *start = NULL;
  RMUtil_ParseArgsAfter("START", argv, argc, "s", &start);
  if (start == NULL && argc == 3) {
    start = argv[2];
  }
  if (start == NULL) {
    RedisModule_Log(ctx, "warning", "No start id found");
    return RedisModule_ReplyWithError(ctx, "NEED START");
  }

  RedisModuleCallReply *rep = RedisModule_Call(ctx, "XREAD", "clclcss", "BLOCK", 3000, "COUNT", 100, "STREAMS", user_stream_key, start);
  RMUTIL_ASSERT_NOERROR(ctx, rep)
  return RedisModule_ReplyWithCallReply(ctx, rep);
}

/**
 * IM.SEND [<uid>]
 */

int SendCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  return REDISMODULE_ERR;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx) {

  // Register the module itself
  if (RedisModule_Init(ctx, "im", 1, REDISMODULE_APIVER_1) ==
      REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  // register im.get - the default registration syntax
  if (RedisModule_CreateCommand(ctx, "im.recive", ReciveCommand, "readonly",
                                1, 1, 1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  // register im.send - using the shortened utility registration macro
  RMUtil_RegisterWriteDenyOOMCmd(ctx, "im.send", SendCommand);

  return REDISMODULE_OK;
}
