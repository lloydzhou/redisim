

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
### 单聊
1. 用户的stream增加group
2. 用户将consumer挂载到group下面
3. 用户读取消息逻辑：
a. 先从consumer读取未读消息列表
b. 再从`c:<uid>`读取联系人列表（这个是否真的有必要？）
c. 直接从`s:<uid>`倒序读取消息，如果在未读消息列表，就标记未读


### 群聊
TODO 

## 历史消息

### 群聊
1. 直接`XREAD`读取`gs:<gid>`

### 单聊
1. 倒序遍历`s:<uid>`，这里遍历自己的消息，如果初始化的时候已经有一些最近的消息了，就使用`XREVRANGE`通过传递`end id`倒序读取
2. 过滤对应发送方的消息



