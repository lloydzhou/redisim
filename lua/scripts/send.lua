local uid = KEYS[1]
local tuid = KEYS[2]
local mid = redis.call('XADD', 's:' .. tuid, '*', unpack(ARGV))
redis.call('XADD', 's:' .. uid, mid, 'FW', 's:' .. tuid)
redis.call('ZADD', 'r:' .. tuid, 'INCR', 1, uid)
return mid

