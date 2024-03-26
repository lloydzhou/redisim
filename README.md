

# 使用redis stream实现im

## redis key

1. `u:<uid>` hash, 存放用户信息，例如name/avatar等
2. `s:<uid>` stream, 这个队列用于接收/发送用户消息
3. `c:<uid>` sorted set, 用于存放用户联系人（包括群组）
4. `gm:<gid>` set, 用于存放群用户id
5. `gs:<gid>` stream, 用于接收/发送群消息
6. `gi:<gid>` hash, 存放群信息，例如name，还有一些配置信息（例如加群是否需要验证？）

## redis module
> redis module实现IM

### IM.RECIVE
> IM.RECIVE [uid] BLOCK [ms] COUNT [count] START [start]  
> 获取历史消息，以及监听新的消息

### IM.MESSAGE
> IM.RECIVE [GROUP | USER] [uid | gid] [mid]  
> 获取单条消息

### IM.SEND
> IM.SEND [uid] [tuid] [field value] [field value ... ]  
> 发送单聊消息

### IM.GSEND
> IM.GSEND [uid] [gid] [field value] [field value ... ]  
> 发送群聊消息

### IM.USER
> IM.USER  [uid]  
> (get user info)  获取用户信息  
> IM.USER  [uid] [field value] [field value ... ]  
> (create user or update user info)

### IM.GROUP
> IM.GROUP [gid]  
> (get group info)  
> IM.GROUP [gid] [uid] [field value] [field value ... ]  
> (create group or update group info)

### IM.LINK
> IM.LINK [uid] [tuid]  
> (add to user contact list)

### IM.UNLINK
> IM.UNLINK [uid] [tuid]  
> (remove from user contact list)

### IM.JOIN
> IM.LINK [uid] [gid]  
> (add to user group)

### IM.QUIT
> IM.QUIT [uid] [gid]  
> (remove from user group)

## GUI
1. using tornado WebSocketHandler to create websocket.
2. using svelte create simple ui to send message (using localStorage to save message)


## docker

```
docker run --rm --name=redisim -p 8888:8888 lloydzhou/redisim
```

# python sdk
```
pip install redisim
```
## demo
```
import asyncio
import redis.asyncio as redis
import redisim

async def main():
    client = redis.Redis(decode_responses=True)
    res = await client.im().recive(user_id, **kwargs)
    print(res)

asyncio.run(main())
```

## lua script version
```
im = client.im(module=False)
```

## apache/kvrocks

using lua script version with apache/kvrocks

