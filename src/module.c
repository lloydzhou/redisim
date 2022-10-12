#define REDISMODULE_EXPERIMENTAL_API
#include <time.h>
#include <math.h>
#include "./redismodule.h"
#include "./rmutil/util.h"
#include "./rmutil/strings.h"
#include "./rmutil/test_util.h"

/**
 * IM.RECIVE  [<uid>] BLOCK [ms] COUNT [count] START [start]
 * start 0-0
 */
int recive_reply_process(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 2 || argc > 8) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModuleString *user_stream_key = RedisModule_CreateStringPrintf(ctx, "s:%s", RedisModule_StringPtrLen(argv[1], NULL));

  long long count = 20;
  RMUtil_ParseArgsAfter("COUNT", argv, argc, "l", &count);

  RedisModuleString *start = NULL;
  RMUtil_ParseArgsAfter("START", argv, argc, "s", &start);
  if (start == NULL) {
    RedisModule_Log(ctx, "warning", "No start id found");
    return RedisModule_ReplyWithError(ctx, "NEED START");
  }
  RedisModuleCallReply *rep = RedisModule_Call(ctx, "XREAD", "clcss", "COUNT", count, "STREAMS", user_stream_key, start);
  // RMUTIL_ASSERT_NOERROR(ctx, rep);
  RedisModule_Log(ctx, "warning", "rep type %d", RedisModule_CallReplyType(rep));
  if ((rep != NULL) && (RedisModule_CallReplyType(rep) != REDISMODULE_REPLY_NULL)) {
    return RedisModule_ReplyWithCallReply(ctx, rep);
  }
  return REDISMODULE_ERR;
}
int recive_reply_callback(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  RedisModuleString *keyname = RedisModule_GetBlockedClientReadyKey(ctx);
  RedisModule_Log(ctx, "warning", "recive_reply_callback keyname %s %d", RedisModule_StringPtrLen(keyname, NULL), argc);
  if(recive_reply_process(ctx, argv, argc) != REDISMODULE_OK) {
    RedisModule_ReplyWithNull(ctx);
  }
  return REDISMODULE_OK;
}
int recive_timeout_callback(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  return RedisModule_ReplyWithNull(ctx);
}

int ReciveCommandKey(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 2 || argc > 8) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModuleString *user_stream_key = RedisModule_CreateStringPrintf(ctx, "s:%s", RedisModule_StringPtrLen(argv[1], NULL));

  long long block = 10000;
  RMUtil_ParseArgsAfter("BLOCK", argv, argc, "l", &block);

  int res = recive_reply_process(ctx, argv, argc);
  if (res != REDISMODULE_OK && block > 0) {
    RedisModule_BlockClientOnKeys(ctx, recive_reply_callback, recive_timeout_callback, NULL, block, &user_stream_key, 1, NULL);
  }

  return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx) {

  // Register the module itself
  if (RedisModule_Init(ctx, "im", 1, REDISMODULE_APIVER_1) ==
      REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  // register im.get - the default registration syntax
  if (RedisModule_CreateCommand(ctx, "im.recive", ReciveCommandKey, "", 0, 0, 0) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  // register im.send - using the shortened utility registration macro
  // RMUtil_RegisterWriteDenyOOMCmd(ctx, "im.send", SendCommand);


  return REDISMODULE_OK;
}
