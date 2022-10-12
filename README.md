

# 使用redis stream实现im

## redis key

1. `u:<uid>` hash, 存放用户信息，例如name/avatar等
2. `s:<uid>` stream, 这个队列用于接收/发送用户消息
3. `c:<uid>` sorted set, 用于存放用户联系人（包括群组）
4. `gm:<gid>` set, 用于存放群用户id
5. `gs:<gid>` stream, 用于接收/发送群消息
6. `gi:<gid>` hash, 存放群信息，例如name，还有一些配置信息（例如加群是否需要验证？）

## consumer

1. group consumer --> `gs:<gid>` 监听群消息，将群消息转发到`s:<uid>`
> 考虑到多个群需要多个消费者的情况：
a. 全局使用一个统一的`g:all`队列
b. 给`g:all`配置一个`g:group`消费组
c. 可以开启多个队列挂到`g:group`下面，将对应的消息发到群里，同时将消息分发到多个用户的队列
d. `g:all`消费者只将原始消息体转存`gs:<gid>`，用户队列使用和`gs:<gid>`相同的消息`id`但是不存消息体，只存`group <gid>`一个字段
> 之所以只将原始消息存`gs:<gid>`一方面是节省存储空间，另一方面是原始消息放`gs:<gid>`入群的用户实际上可以查看群内历史聊天记录。

2. user consumer --> `s:<uid>` 客户端监听用户消息队列
> 读取用户队列消息的时候，如果消息只有一个`group <gid>`字段，则到`gs:<gid>`按照`id`读取原始消息内容发给客户端


## 消息已读

### 未读消息粒度只到联系人级别
1. 使用`r:<uid>` sorted set, 存放用户未读消息，如果全部已读`score`就是0
2. 未读消息列表基本和联系人列表数据结构一致


### ~~未读消息粒度到消息级别~~
#### ~~单聊~~
1. 用户的stream增加group
2. 用户将consumer挂载到group下面
3. 用户读取消息逻辑：
a. 先从consumer读取未读消息列表
b. 再从`c:<uid>`读取联系人列表（这个是否真的有必要？）
c. 直接从`s:<uid>`倒序读取消息，如果在未读消息列表，就标记未读


#### ~~群聊~~
实际上群聊消息也会出现在用户自己的队列里面


## 历史消息

### 群聊
1. 直接`XREAD`读取`gs:<gid>`

### 单聊
1. 倒序遍历`s:<uid>`，这里遍历自己的消息，如果初始化的时候已经有一些最近的消息了，就使用`XREVRANGE`通过传递`end id`倒序读取
2. 过滤对应发送方的消息
3. 感觉这个可能会有性能问题，是不是考虑直接在创建连接的时候直接同步用户最近的信息就好了，前端收到消息就按照用户的发送方或者接收方，直接在前端和联系人列表一起组织好整个结构。


## 接口
### /ws
1. 创建连接的时候带上token，获取user_id
2. 获取用户联系人列表发送给前端，先发送联系人列表，后续接收消息的时候才好进行展示
3. 然后异步倒序获取消息（如果是自己发送的消息通过`user`字段从其他用户队列获取原始消息，如果是群消息通过`group`字段从群队列获取原始消息）
4. 同时开启一个while循环以block模式监听最新消息，如果接收到最新消息就发送给前端（接收到的消息也走相同的逻辑拿原始消息）
5. 前端直接通过websocket发送消息，如果是单聊消息，往对方队列存原始消息，同时在自己队列存一个不包含原始消息的消息体。
6. 如果是群聊消息，往

### 动作
> 抽象一些动作（差不多也对应一个接口动作）
> 往统一消息中心g:all（后续改成mcenter）发消息触发动作
1. 初始化用户（创建用户）
```
create_user(uid?: string, name: string, avatar?: string, **kwargs)

HMSET u:<uid> name <name> avatar <avatar> ...
// c:<uid>, 和r:<uid> 这两个sorted set可以不用提前创建
```
2. 添加联系人
```
contact(uid: string, tuid: string)

ZADD c:<uid> timestamp target
ZADD r:<uid> 0 target

send(tuid, uid, action='contact', uid="<uid>", tuid="<tuid>")
```
3. 发送单聊消息
```
send(uid: string, tuid: string, **kwargs)

// 真实的消息内容
mid = XADD s:<tuid> * <**kwargs>
// 未读消息+1
ZADD r:<tuid> INCR 1 "<uid>"
// 代表是从另一个队列转发的消息，在另一个队列的id是mid
// 前端看起来就相当于展示我发送出去的消息
XADD s:<uid> mid FW "s:<tuid>"
```

4. 创建群
```
create_group(uid: string, gid: string, name: string, avatar: string)
HMSET gi:<gid> master "<uid>" name "<name>" avatar "<avatar>"

join(gid, uid)
```

5. 加入群
```
join(gid: string, uid: string)

// 加联系人
ZADD u:<uid> timestamp gid
ZADD r:<uid> 0 gid

// 加组
SADD gm:<gid> uid
// 群发通知消息
gsend(uid, gid, action='join', group="<gid>")
// XADD gs:<gid> * user "<uid>" action "join" group "<gid>"
```

6. 发送群聊消息
```
gsend(uid, gid, **kwargs)
// 群内发消息
mid = XADD gs:<gid> * user "<uid>" (**kwargs)
// 同时将消息转发给群内用户
for u in (SMEMBERS gm:<gid>):
  XADD s:<u> mid FW "gs:<gid>"
  ZADD r:<u> INCR 1 "gs:<gid>"
```

7. 退群
```
quit(uid, gid)

SREM gm:<gid> uid
// 群发通知消息
gsend(uid, gid, action='quit', group="<gid>")
// XADD gs:<gid> * user "<uid>" action "quit" group "<gid>"
```

8. 删好友
```
ulink(uid, tuid)

ZREM c:<uid> tuid

// 不发消息?发消息可以提醒前端删除当前联系人
send(tuid, uid, action='unlink', uid="<uid>", tuid="<tuid>")
```

9. 获取历史消息
```
history(uid, last="+")

// 100条为单位倒序获取消息
last = "+"
while true:
  messages = XREVRANGE s:<uid> <last> - COUNT 100
  if len(messages) == 0:
    break
  for message in messages
    last = message.id
    if message.FW:
      // 获取原始消息内容
      m = XREAD COUNT 1 STREAMS <message.FW> message.id
      write(m)
    else:
      write(message)
    end
  end
end
```
9.1 获取新消息
> 客户端有缓存消息的时候，使用一个之前的消息id开始读取后续消息
```
recive(uid, start="$")

while true:
  messages = XREAD BLOCK 3000 COUNT 100 STREAMS s:<uid> start
  for message in messages
    // 这里同前面的逻辑
    write(get_message(message))
  end
end
```
10. 获取个人信息
```
user(uid)

HGETALL u:<uid>
```
11. 获取群信息
```
group(gid)

HGETALL gi:<gid>
SMEMBERS gm:<gid>
```
12. 消息已读
```
read(uid, tuid|gid)

ZADD 0 uid <tuid|gid>
```


### lua script
TODO
前面的每一个动作几乎都可以写成一个lua函数
调用lua函数，减少和redis之间通信次数是比较高效的
使用lua脚本还可以减少设计那个消费者

### redis module
1. recive需要block api，在module里面实现block api看起来不是很好处理
2. send和gsend需要调用XADD，在C语言里面不是很好处理变长参数透传逻辑。

1. 可以通过RedisModule_BlockClient实现
1.1 需要配合RedisModule_SubscribeToKeyspaceEvents, block client的时候将当前的client以及key放到一个队列中，然后监听到stream的xadd事件，再拿到对应的client返回数据
2. 可以通过传递一个argv和argc实现

## 服务
1. 一个websocket server
2. 同时启动一个`g:all`消费者负责转发消息



