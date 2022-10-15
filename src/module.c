#define REDISMODULE_EXPERIMENTAL_API
#include "./redismodule.h"
#include "./rmutil.c"

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
  if ((rep != NULL) && (RedisModule_CallReplyType(rep) != REDISMODULE_REPLY_NULL)) {
    return RedisModule_ReplyWithCallReply(ctx, rep);
  }
  return REDISMODULE_ERR;
}
int recive_reply_callback(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  // RedisModuleString *keyname = RedisModule_GetBlockedClientReadyKey(ctx);
  // RedisModule_Log(ctx, "warning", "recive_reply_callback keyname %s %d", RedisModule_StringPtrLen(keyname, NULL), argc);
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

// only process logic not reply
RedisModuleCallReply* _GroupSendCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  // mid = XADD gs:<gid> * user "<uid>" (**kwargs)
  // IM.GSEND  [<uid>] [<gid>] [field value] [field value ... ]
  RedisModuleString **args = calloc(argc+1, sizeof(RedisModuleString*));

  args[0] = RedisModule_CreateStringPrintf(ctx, "gs:%s", RedisModule_StringPtrLen(argv[2], NULL));
  args[1] = RedisModule_CreateStringPrintf(ctx, "*");
  args[2] = RedisModule_CreateStringPrintf(ctx, "uid");
  args[3] = argv[1];
  for (int i=3; i<argc; i++){
    args[i+1] = RedisModule_CreateStringFromString(ctx, argv[i]);
  }
  RedisModuleCallReply* rep = RedisModule_Call(ctx, "XADD", "v", args, argc + 1);
  RedisModuleString *mid = RedisModule_CreateStringFromCallReply(rep);

  RedisModuleString *gmid = RedisModule_CreateStringPrintf(ctx, "gm:%s", RedisModule_StringPtrLen(argv[2], NULL));
  RedisModuleCallReply* mrep = RedisModule_Call(ctx, "SMEMBERS", "s", gmid);
  RedisModuleString *u = NULL;
  RedisModuleString *ui = NULL;
  // for u in (SMEMBERS gm:<gid>):
  //   XADD s:<u> mid FW "gs:<gid>"
  //   ZADD r:<u> INCR 1 "gs:<gid>"
  for (int i = 0; i < RedisModule_CallReplyLength(mrep); i++) {
    u = RedisModule_CreateStringFromCallReply(RedisModule_CallReplyArrayElement(mrep, i));
    ui = RedisModule_CreateStringPrintf(ctx, "s:%s", RedisModule_StringPtrLen(u, NULL));
    // RedisModule_Log(ctx, "warning", "FW %s %s", RedisModule_StringPtrLen(ui, NULL), RedisModule_StringPtrLen(mid, NULL));
    RedisModule_Call(ctx, "XADD", "sscs", ui, mid, "FW", args[0]);
    if (!RMUtil_StringEquals(u, argv[1])) {
      // redis.call('ZADD', 'r:' .. u, 'INCR', 1, gs:<gid>)
      RedisModuleString *rtuid = RedisModule_CreateStringPrintf(ctx, "r:%s", RedisModule_StringPtrLen(u, NULL));
      RedisModule_Call(ctx, "ZADD", "scls", rtuid, "INCR", 1, args[0]);
    }
  }
  return rep;
}

/**
 * IM.GSEND  [<uid>] [<gid>] [field value] [field value ... ]
 */
int GroupSendCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 5) {
    return RedisModule_WrongArity(ctx);
  }
  if (argc % 2 == 0) {
    return RedisModule_WrongArity(ctx);
  }

  RedisModule_AutoMemory(ctx);

  RedisModuleCallReply *rep = _GroupSendCommand(ctx, argv, argc);
  return RedisModule_ReplyWithCallReply(ctx, rep);
}

RedisModuleCallReply* _SendCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  // local mid = redis.call('XADD', 's:' .. tuid, '*', unpack(ARGV))
  RedisModuleString **args = calloc(argc+1, sizeof(RedisModuleString*));

  args[0] = RedisModule_CreateStringPrintf(ctx, "s:%s", RedisModule_StringPtrLen(argv[2], NULL));
  args[1] = RedisModule_CreateStringPrintf(ctx, "*");
  args[2] = RedisModule_CreateStringPrintf(ctx, "uid");
  args[3] = argv[1];
  for (int i=3; i<argc; i++){
    args[i+1] = RedisModule_CreateStringFromString(ctx, argv[i]);
  }
  RedisModuleCallReply* rep = RedisModule_Call(ctx, "XADD", "v", args, argc + 1);

  // redis.call('XADD', 's:' .. uid, mid, 'FW', 's:' .. tuid)
  RedisModuleString *uid = RedisModule_CreateStringPrintf(ctx, "s:%s", RedisModule_StringPtrLen(argv[1], NULL));
  RedisModuleString *mid = RedisModule_CreateStringFromCallReply(rep);
  // RedisModuleCallReply *fwrep = RedisModule_Call(ctx, "XADD", "sscs", uid, mid, "FW", args[0]);
  RedisModule_Call(ctx, "XADD", "sscs", uid, mid, "FW", args[0]);

  // redis.call('ZADD', 'r:' .. tuid, 'INCR', 1, s:<uid>)
  RedisModuleString *rtuid = RedisModule_CreateStringPrintf(ctx, "r:%s", RedisModule_StringPtrLen(argv[2], NULL));
  // RedisModuleCallReply* zrep = RedisModule_Call(ctx, "ZADD", "s", rtuid, "INCR", 1, argv[1]);
  RedisModule_Call(ctx, "ZADD", "scls", rtuid, "INCR", 1, uid);

  // return mid
  return rep;
}
/**
 * IM.SEND  [<uid>] [<tuid>] [field value] [field value ... ]
 */
int SendCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 5) {
    return RedisModule_WrongArity(ctx);
  }
  if (argc % 2 == 0) {
    return RedisModule_WrongArity(ctx);
  }

  RedisModule_AutoMemory(ctx);

  RedisModuleCallReply *rep = _SendCommand(ctx, argv, argc);

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

RedisModuleCallReply* _LinkCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  // add use concat
  // ZADD c:<uid> timestamp tuid
  // ZADD r:<uid> 0 tuid (no need)
  long long ms = RedisModule_Milliseconds();
  RedisModuleString *cuid = RedisModule_CreateStringPrintf(ctx, "c:%s", RedisModule_StringPtrLen(argv[1], NULL));
  return RedisModule_Call(ctx, "ZADD", "scls", cuid, "NX", ms, argv[2]);
}
/**
 * IM.LINK [uid] [tuid]
 */
int LinkCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  RedisModule_AutoMemory(ctx);
  if (argc != 3) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModuleCallReply* rep = _LinkCommand(ctx, argv, argc);
  if (RedisModule_CallReplyInteger(rep) == 0) {
    // already in concat list
    return RedisModule_ReplyWithCallReply(ctx, rep);
  }
  // IM.SEND  [<uid>] [<tuid>] [action link] [uid <uid>] [tuid <tuid>]
  RedisModuleString **sargs = calloc(9, sizeof(RedisModuleString*));
  // jargs[0] = "IM.SEND"
  sargs[1] = sargs[6] = RedisModule_CreateStringFromString(ctx, argv[1]);
  sargs[2] = sargs[8] = RedisModule_CreateStringFromString(ctx, argv[2]);
  sargs[3] = RedisModule_CreateStringPrintf(ctx, "action");
  sargs[4] = RedisModule_CreateStringPrintf(ctx, "link");
  sargs[5] = RedisModule_CreateStringPrintf(ctx, "uid");
  sargs[7] = RedisModule_CreateStringPrintf(ctx, "tuid");
  _SendCommand(ctx, sargs, 9);
  return RedisModule_ReplyWithCallReply(ctx, rep);
}

RedisModuleCallReply* _UnLinkCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  // ulink(uid, tuid)
  // ZREM c:<uid> tuid
  // ZREM r:<uid> s:tuid
  RedisModuleString *cuid = RedisModule_CreateStringPrintf(ctx, "c:%s", RedisModule_StringPtrLen(argv[1], NULL));
  RedisModuleCallReply *rep = RedisModule_Call(ctx, "ZREM", "ss", cuid, argv[2]);
  RedisModuleString *ruid = RedisModule_CreateStringPrintf(ctx, "c:%s", RedisModule_StringPtrLen(argv[1], NULL));
  RedisModuleString *stuid = RedisModule_CreateStringPrintf(ctx, "c:%s", RedisModule_StringPtrLen(argv[2], NULL));
  RedisModule_Call(ctx, "ZREM", "ss", ruid, stuid);
  return rep;
}
/**
 * IM.UNLINK [uid] [tuid]
 */
int UnLinkCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  RedisModule_AutoMemory(ctx);
  if (argc != 3) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModuleCallReply *rep = _UnLinkCommand(ctx, argv, argc);
  return RedisModule_ReplyWithCallReply(ctx, rep);
}

// process
RedisModuleCallReply* _JoinGroupCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  RedisModuleString *gmid = RedisModule_CreateStringPrintf(ctx, "gm:%s", RedisModule_StringPtrLen(argv[2], NULL));
  // exists in group
  // RedisModuleCallReply *e = RedisModule_Call(ctx, "SISMEMBER", "ss", gmid, argv[1]);
  // if ((e != NULL) && (RedisModule_CallReplyType(e) == REDISMODULE_REPLY_INTEGER) && (RedisModule_CallReplyInteger(e) == 1)) {
  //   return e;
  // }
  // add use concat
  // ZADD c:<uid> timestamp gid
  // ZADD r:<uid> 0 gid
  _LinkCommand(ctx, argv, argc);
  // add use to group
  RedisModuleCallReply *rep = RedisModule_Call(ctx, "SADD", "ss", gmid, argv[1]);
  if (RedisModule_CallReplyInteger(rep) == 0) {
    // already in group set
    return rep;
  }
  // send join group message
  // IM.GSEND  [<uid>] [<gid>] [action join] [uid <uid>] [group <gid>]
  RedisModuleString **gargs = calloc(9, sizeof(RedisModuleString*));
  // gargs[0] = "IM.GSEND"
  gargs[1] = gargs[6] = RedisModule_CreateStringFromString(ctx, argv[1]);
  gargs[2] = gargs[8] = RedisModule_CreateStringFromString(ctx, argv[2]);
  gargs[3] = RedisModule_CreateStringPrintf(ctx, "action");
  gargs[4] = RedisModule_CreateStringPrintf(ctx, "join");
  gargs[5] = RedisModule_CreateStringPrintf(ctx, "uid");
  gargs[7] = RedisModule_CreateStringPrintf(ctx, "gid");
  _GroupSendCommand(ctx, gargs, 9);
  return rep;
}
/**
 * IM.JOIN [uid] [gid]
 */
int JoinGroupCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  //
  RedisModule_AutoMemory(ctx);
  if (argc != 3) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModuleCallReply *rep = _JoinGroupCommand(ctx, argv, argc);
  return RedisModule_ReplyWithCallReply(ctx, rep);
}

/**
 * IM.QUIT [uid] [gid]
 */
int QuitGroupCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  RedisModule_AutoMemory(ctx);
  if (argc != 3) {
    return RedisModule_WrongArity(ctx);
  }
  // remove gid
  _UnLinkCommand(ctx, argv, argc);

  // SREM gm:<gid> tuid
  // ZREM r:<uid> s:tuid
  RedisModuleString *gmid = RedisModule_CreateStringPrintf(ctx, "gm:%s", RedisModule_StringPtrLen(argv[2], NULL));
  RedisModuleCallReply *rep = RedisModule_Call(ctx, "SREM", "ss", gmid, argv[1]);
  // send join group message
  // IM.GSEND  [<uid>] [<gid>] [action quit] [uid <uid>] [group <gid>]
  RedisModuleString **gargs = calloc(9, sizeof(RedisModuleString*));
  // gargs[0] = "IM.GSEND"
  gargs[1] = gargs[6] = RedisModule_CreateStringFromString(ctx, argv[1]);
  gargs[2] = gargs[8] = RedisModule_CreateStringFromString(ctx, argv[2]);
  gargs[3] = RedisModule_CreateStringPrintf(ctx, "action");
  gargs[4] = RedisModule_CreateStringPrintf(ctx, "join");
  gargs[5] = RedisModule_CreateStringPrintf(ctx, "uid");
  gargs[7] = RedisModule_CreateStringPrintf(ctx, "gid");
  _GroupSendCommand(ctx, gargs, 9);
  return RedisModule_ReplyWithCallReply(ctx, rep);
}

/**
 * IM.GROUP [gid] (get group info)
 * IM.GROUP [gid] [uid] [field value] [field value ... ] (create user)
 */
int GroupCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  RedisModule_AutoMemory(ctx);
  if (argc < 2) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModuleString *gid = RedisModule_CreateStringPrintf(ctx, "gi:%s", RedisModule_StringPtrLen(argv[2], NULL));
  if (argc == 2) {
    // get group info
    // HGETALL gi:<gid>
    // SMEMBERS gm:<gid>
    RedisModuleString *gmid = RedisModule_CreateStringPrintf(ctx, "gm:%s", RedisModule_StringPtrLen(argv[2], NULL));
    RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
    RedisModule_ReplyWithCallReply(ctx, RedisModule_Call(ctx, "HGETALL", "s", gid));
    RedisModule_ReplyWithCallReply(ctx, RedisModule_Call(ctx, "SMEMBERS", "s", gmid));
    RedisModule_ReplySetArrayLength(ctx, 2);
    return REDISMODULE_OK;
  }
  if (argc < 3) {
    return RedisModule_WrongArity(ctx);
  }
  if (argc % 2 == 0) {
    return RedisModule_WrongArity(ctx);
  }

  // redis.call('HMSET', 'gi:' .. gid, 'master', uid, unpack(ARGV))
  RedisModuleString **args = calloc(argc, sizeof(RedisModuleString*));

  args[0] = gid;
  args[1] = RedisModule_CreateStringPrintf(ctx, "master");
  args[2] = RedisModule_CreateStringFromString(ctx, argv[1]);
  for (int i=3; i<argc; i++){
    args[i] = RedisModule_CreateStringFromString(ctx, argv[i]);
  }
  RedisModuleCallReply* rep = RedisModule_Call(ctx, "HMSET", "v", args, argc);
  // join(uid: string, gid: string)
  RedisModuleString **jargs = calloc(3, sizeof(RedisModuleString*));
  // jargs[0] = "IM.JOIN"
  jargs[1] = RedisModule_CreateStringFromString(ctx, argv[1]);
  jargs[2] = RedisModule_CreateStringFromString(ctx, argv[2]);
  _JoinGroupCommand(ctx, jargs, 3);
  return RedisModule_ReplyWithCallReply(ctx, rep);
}

/**
 * IM.MESSAGE [GROUP | USER] [uid] [mid] (get one message)
 */
int MessageCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 2) {
    return RedisModule_WrongArity(ctx);
  }
  RedisModule_AutoMemory(ctx);

  RedisModuleString *gid = NULL;
  RMUtil_ParseArgsAfter("GROUP", argv, argc, "s", &gid);
  RedisModuleString *uid = NULL;
  RMUtil_ParseArgsAfter("USER", argv, argc, "s", &uid);

  if (gid == NULL && uid == NULL && argc != 3) {
    return RedisModule_WrongArity(ctx);
  }
  if ((gid != NULL || uid != NULL) && argc != 4) {
    return RedisModule_WrongArity(ctx);
  }

  // last param is mid
  RedisModuleString *mid = argv[argc - 1];

  if (gid == NULL && uid == NULL) {
    uid = argv[1];
  }
  RedisModuleString *channel_id = RedisModule_CreateStringPrintf(ctx, uid == NULL ? "gs:%s" : "s:%s", RedisModule_StringPtrLen(uid == NULL ? gid : uid, NULL));

  RedisModuleCallReply* rep = RedisModule_Call(ctx, "XRANGE", "sss", channel_id, mid, mid);
  if (RedisModule_CallReplyLength(rep) == 1) {
    return RedisModule_ReplyWithCallReply(ctx, RedisModule_CallReplyArrayElement(rep, 0));
  } else {
    return RedisModule_ReplyWithNull(ctx);
  }
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

  if (RedisModule_CreateCommand(ctx, "im.gsend", GroupSendCommand, "", 0, 0, 0) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "im.user", UserCommand, "", 0, 0, 0) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "im.group", GroupCommand, "", 0, 0, 0) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "im.link", LinkCommand, "", 0, 0, 0) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "im.unlink", UnLinkCommand, "", 0, 0, 0) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "im.join", JoinGroupCommand, "", 0, 0, 0) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "im.quit", QuitGroupCommand, "", 0, 0, 0) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  if (RedisModule_CreateCommand(ctx, "im.message", MessageCommand, "", 0, 0, 0) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  return REDISMODULE_OK;
}
