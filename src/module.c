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

  RedisModule_AutoMemory(ctx);

  RedisModuleString *user_stream_key = RedisModule_CreateStringPrintf(ctx, "s:%s", RedisModule_StringPtrLen(argv[1], NULL));

  long long block = 10000;
  RMUtil_ParseArgsAfter("BLOCK", argv, argc, "l", &block);

  int res = recive_reply_process(ctx, argv, argc);
  if (res != REDISMODULE_OK && block > 0) {
    RedisModule_BlockClientOnKeys(ctx, recive_reply_callback, recive_timeout_callback, NULL, block, &user_stream_key, 1, NULL);
  }

  return REDISMODULE_OK;
}

/**
 * IM.SEND  [<uid>] [<tuid>] [field value] [field value ... ]
 */
int SendCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 3) {
    return RedisModule_WrongArity(ctx);
  }
  if (argc % 2 == 0) {
    return RedisModule_WrongArity(ctx);
  }

  RedisModule_AutoMemory(ctx);

  // local mid = redis.call('XADD', 's:' .. tuid, '*', unpack(ARGV))
  RedisModuleString **args = calloc(argc-1, sizeof(RedisModuleString*));

  args[0] = RedisModule_CreateStringPrintf(ctx, "s:%s", RedisModule_StringPtrLen(argv[2], NULL));
  args[1] = RedisModule_CreateStringPrintf(ctx, "*");
  for (int i=3; i<argc; i++){
    args[i-1] = RedisModule_CreateStringFromString(ctx, argv[i]);
  }
  RedisModuleCallReply* rep = RedisModule_Call(ctx, "XADD", "v", args, argc - 1);

  // redis.call('XADD', 's:' .. uid, mid, 'FW', 's:' .. tuid)
  RedisModuleString *uid = RedisModule_CreateStringPrintf(ctx, "s:%s", RedisModule_StringPtrLen(argv[1], NULL));
  RedisModuleString *mid = RedisModule_CreateStringFromCallReply(rep);
  // RedisModuleCallReply *fwrep = RedisModule_Call(ctx, "XADD", "sscs", uid, mid, "FW", args[0]);
  RedisModule_Call(ctx, "XADD", "sscs", uid, mid, "FW", args[0]);

  // redis.call('ZADD', 'r:' .. tuid, 'INCR', 1, uid)
  RedisModuleString *rtuid = RedisModule_CreateStringPrintf(ctx, "r:%s", RedisModule_StringPtrLen(argv[2], NULL));
  // RedisModuleCallReply* zrep = RedisModule_Call(ctx, "ZADD", "s", rtuid, "INCR", 1, argv[1]);
  RedisModule_Call(ctx, "ZADD", "scls", rtuid, "INCR", 1, argv[1]);

  // return mid
  return RedisModule_ReplyWithCallReply(ctx, rep);
}

/**
 * IM.USER  [uid] (get user info)
 * IM.USER  [<uid>] [field value] [field value ... ] (create user)
 */
int UserCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 2) {
    return RedisModule_WrongArity(ctx);
  }
  if (argc % 2 == 1) {
    return RedisModule_WrongArity(ctx);
  }

  RedisModule_AutoMemory(ctx);

  // local mid = redis.call('XADD', 's:' .. tuid, '*', unpack(ARGV))
  RedisModuleString **args = calloc(argc-1, sizeof(RedisModuleString*));

  args[0] = RedisModule_CreateStringPrintf(ctx, "u:%s", RedisModule_StringPtrLen(argv[1], NULL));
  if (argc == 2) {
    return RedisModule_ReplyWithCallReply(ctx, RedisModule_Call(ctx, "HGETALL", "s", args[0]));
  }
  for (int i=2; i<argc; i++){
    args[i-1] = RedisModule_CreateStringFromString(ctx, argv[i]);
  }
  RedisModuleCallReply* rep = RedisModule_Call(ctx, "HMSET", "v", args, argc - 1);
  return RedisModule_ReplyWithCallReply(ctx, rep);
}

/**
 * IM.GROUP  [uid] (get user info)
 * IM.GROUP  [<uid>] [field value] [field value ... ] (create user)
 */
int GroupCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 2) {
    return RedisModule_WrongArity(ctx);
  }
  if (argc % 2 == 1) {
    return RedisModule_WrongArity(ctx);
  }

  RedisModule_AutoMemory(ctx);

  // local mid = redis.call('XADD', 's:' .. tuid, '*', unpack(ARGV))
  RedisModuleString **args = calloc(argc-1, sizeof(RedisModuleString*));

  args[0] = RedisModule_CreateStringPrintf(ctx, "u:%s", RedisModule_StringPtrLen(argv[1], NULL));
  if (argc == 2) {
    return RedisModule_ReplyWithCallReply(ctx, RedisModule_Call(ctx, "HGETALL", "s", args[0]));
  }
  for (int i=2; i<argc; i++){
    args[i-1] = RedisModule_CreateStringFromString(ctx, argv[i]);
  }
  RedisModuleCallReply* rep = RedisModule_Call(ctx, "HMSET", "v", args, argc - 1);
  return RedisModule_ReplyWithCallReply(ctx, rep);
}

int RedisModule_OnLoad(RedisModuleCtx *ctx) {

  // Register the module itself
  if (RedisModule_Init(ctx, "im", 1, REDISMODULE_APIVER_1) ==
      REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "im.recive", ReciveCommandKey, "", 0, 0, 0) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "im.send", SendCommand, "", 0, 0, 0) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "im.user", UserCommand, "", 0, 0, 0) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "im.group", GroupCommand, "", 0, 0, 0) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  return REDISMODULE_OK;
}
