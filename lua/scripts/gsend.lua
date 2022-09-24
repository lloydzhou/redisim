local uid = KEYS[1]
local gid = KEYS[2]
local mid = redis.call('XADD', 'gs:' .. gid, '*', 'user', uid, unpack(ARGV))

local data = redis.call('SMEMBERS', 'gm:' .. gid)
for i=1, #data, 1 do
    redis.call('XADD', 's:' .. data[i], mid, 'FW', 'gs:' .. gid)
    if uid ~= data[i] then
        redis.call('ZADD', 'r:' .. data[i], 'INCR', 1, gid)
    end
end
return mid

