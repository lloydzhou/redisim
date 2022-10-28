import db from 'db.js'
import { writable, derived, get } from './store';

let messagedb

export const events = writable([])
export const messages = writable([])
export const contacts = writable([])
export const conversation = writable([])
export const chats = writable([])
export const user_id = writable('')
export const target_user_id = writable('')
export const group_id = writable('')
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
          key: { keyPath: ['id'] },
          indexes: {
            action: {
              keyPath: ['created', 'message.action']
            },
            created: {
              keyPath: ['created']
            },
            tuid_gid: {
              keyPath: ['tuid', 'gid']
            },
            gid_created: {
              keyPath: ['gid', 'created']
            },
            tuid_created: {
              keyPath: ['tuid', 'created']
            },
          }
        }
      }
    }).then(function (s) {
      messagedb = s.message
      // 初始化messages列表
      // get contacts list from indexeddb
      s.message.query('action')
        .all()
        .execute()
        .then(action => {
          // console.log('action', action)
          const res = []
          const s = new Set()
          action.slice().forEach(i => {
            const { tuid: tu, uid: u } = i
            const { action } = i.message || {}
            if (action == 'link') {
              const tuid = u == su ? tu : u
              if (!s.has(tuid)){
                s.add(tuid)
                res.push({ tuid, message: i })
              }
            }
            else if (action == 'join') {
              if (!s.has(tu)){
                s.add(tu)
                res.push({ tuid: tu, gid: tu, message: i })
              }
            }
          })
          // console.log('contacts', res)
          contacts.set(res)
        })
      // get conversation list from indexeddb
      s.message.query('tuid_gid')
        .all()
        .distinct()
        .execute()
        .then(users => {
          const group = users.filter(i => i.gid).map(i => i.gid);
          const user = Array.from(new Set(users.filter(i => !i.gid).map(i => i.uid == su ? i.tuid : i.uid)))
          // console.log('users', users, group, user)
          return Promise.all(
            group.map(gid => s.message.query('gid_created').filter('gid', gid)).concat(
              user.map(tuid => s.message.query('tuid_created').filter('tuid', tuid))
            ).map(i => i.desc().limit(1).execute())
          ).then(results => {
            const c = results.reduce((s, i) => s.concat(i), []).map((i, n) => {
              return ({tuid: i.tuid, gid: i.gid || users[n].gid, message: i})
            }).sort((a, b) => b.message.created - a.message.created)
            // console.log('conversation', users, c)
            conversation.set(c)
          })
        })

      // get recent messages list
      // set user_id after get last_message
      return s.message.query('created')
        .all()
        .desc()
        .limit(500) // TODO
        .execute()
        .then(results => {
          // console.log('results', results)
          if (results.length > 0) {
            const m = results.slice().reverse()
            last_message.set(m[m.length - 1])
            messages.set(m)
          }
        })
    }).finally(() => {
      user_id.set(su)
    });
  }
} catch (e) {}

last_message.subscribe(message => {
  console.log('last_message', message)
  if (!message.id) {
    return
  }
  // save message to indexeddb
  if (messagedb) {
    messagedb.update(message)
  }
  messages.update(m => m.concat(message))

  const self = get(user_id)
  const { uid, tuid, gid } = message
  const { action } = message.message

  // 添加好友，可能更新contacts或者conversation
  if (action) {
    if (gid) {
      // 自己加群，或者其他人加群
      if (uid == self) {
        contacts.update(c => [{tuid: gid, gid, message}].concat(c))
      }
    } else {
      // 其他人添加好友
      contacts.update(c => [{tuid: tuid != self ? tuid : uid, message}].concat(c))
    }
  }

  // 群消息
  if (gid) {
    // 接收群消息
    conversation.update(c => {
      if (c.filter(i => i.gid == gid).length > 0) {
        return c.map(i => i.gid == gid ? ({ tuid: gid, gid: i.gid || gid, message }) : i).sort((a,b) => b.message.created - a.message.created)
      } else {
        return [{tuid: gid, gid, message}].concat(c)
      }
    })
  } else {
    // 接收个人消息
    const t = tuid != self ? tuid : uid
    conversation.update(c => {
      if (c.filter(i => i.tuid == t).length > 0) {
        return c.map(i => i.tuid == t ? ({ tuid: t, message }) : i).sort((a,b) => b.message.created - a.message.created)
      } else {
        return [{tuid: t, message}].concat(c)
      }
    })
  }

  if (get(target_user_id)) {
    // console.log('update chats', get(chats), get(target_user_id))
    chats.update(c => [message].concat(c))
  }
})

user_id.subscribe((uid) => {
  localStorage.setItem('user_id', uid)
})

group_id.subscribe(gid => {
  console.log('gid', gid, get(chats))
  if (gid && messagedb) {
    messagedb.query('created')
      .filter(i => i.tuid == gid || i.uid == gid)
      .desc()
      .execute()
      .then(results => {
        console.log('chats results', results)
        chats.set(results)
      })
  }
})

target_user_id.subscribe(tuid => {
  console.log('tuid', tuid, get(chats))
  const uid = get(user_id)
  if (uid && tuid && messagedb) {
    messagedb.query('created')
      .filter(i => (i.uid == uid && i.tuid == tuid) || (i.tuid == uid && i.uid == tuid))
      .desc()
      .execute()
      .then(results => {
        console.log('chats results', results)
        chats.set(results)
      })
  }
})

export const unread = derived([messages], ([m]) => {
  return 1
})

conversation.subscribe(c => console.log('conversation', c))

export * from './store'

