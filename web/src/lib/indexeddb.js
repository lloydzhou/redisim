import db from 'db.js'
import { writable, derived, get } from './store';

let messagedb

export const events = writable([])
export const messages = writable([])
export const user_id = writable('')
export const last_message = writable({})
export const last_message_id = derived([last_message], ([m]) => {
  return m.id || ''
})


try {
  let su
  if (su = localStorage.getItem('user_id')) {
    db.open({
      server: 'redisim:' + su,
      version: 1,
      schema: {
        message: {
          key: { keyPath: 'id' },
          indexes: {
            tuid: {
              keyPath: ['tuid', 'created']
            },
          }
        }
      }
    }).then(function (s) {
      messagedb = s.message
      // 初始化messages列表
      s.message.query('tuid')
        .all()
        .desc()
        .limit(500) // TODO
        .execute()
        .then(results => {
          // console.log('results', results)
          messages.set(results)
          if (results.length > 0) {
            last_message.set(results[0])
          }
        })
    }).finally(() => {
      user_id.set(su)
    });
  }
} catch (e) {}

last_message.subscribe(message => {
  console.log('last_message')
  if (messagedb) {
    messagedb.update(message)
  }
})

user_id.subscribe((uid) => {
  localStorage.setItem('user_id', uid)
})

export const target_user_id = writable('')
export const group_id = writable('')
export const conversation = derived([messages, user_id], ([m, uid]) => {
  const res = []
  if (uid) {
    const s = new Set()
    m.slice(0).reverse().forEach(i => {
      const { tuid: tu, uid: u } = i
      const tuid = u == uid ? tu : u
      if (tuid && !s.has(tuid)){
        s.add(tuid)
        res.push({ tuid, message: i })
      }
    })
  }
  return res
})
export const contacts = derived([messages, user_id], ([m, uid]) => {
  const res = []
  if (uid) {
    const s = new Set()
    m.slice().reverse().forEach(i => {
      const { tuid: tu, uid: u } = i
      const { action } = i.message || {}
      if (action == 'link') {
        const tuid = u == uid ? tu : u
        if (!s.has(tuid)){
          s.add(tuid)
          res.push({ tuid, message: i })
        }
      }
      else if (action == 'join') {
        if (!s.has(tu)){
          s.add(tu)
          res.push({ tuid: tu, message: i })
        }
      }
    })
  }
  return res
})
export const chats = derived([messages, user_id, target_user_id, group_id], ([m, uid, tuid, gid]) => {
  if (tuid == uid) {
    return m.filter(t => t.message && t.tuid == tuid && t.uid == tuid)
  }
  if (gid) {
    return m.filter(t => t.message && (t.tuid == gid || t.uid == gid))
  }
  if (tuid) {
    return m.filter(t => t.message && ((t.tuid == tuid && t.uid == uid) || (t.uid == tuid && t.tuid == uid)) || t.gid == tuid);
  }
  return []
})
export const unread = derived([messages], ([m]) => {
  return 1
})

export * from './store'

