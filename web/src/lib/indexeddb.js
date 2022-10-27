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
      server: 'redisim' + su,
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
            tuid: {
              keyPath: ['tuid']
            },
            tuid_created: {
              keyPath: ['tuid', 'created']
            },
          }
        }
      }
    }).catch(err => {
      console.log('err', err)
      throw err
    }).then(function (s) {
      messagedb = s.message
      // 初始化messages列表
      s.message.query('action')
        .all()
        .execute()
        .then(action => {
          console.log('action', action)
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
          contacts.set(res)
        })
      s.message.query('tuid')
        .all()
        .distinct()
        .execute()
        .then(users => Promise.all(users.map(u => s.message.query('tuid_created').filter('tuid', u.tuid).desc().limit(1).execute())).then(results => {
          const c = results.reduce((s, i) => s.concat(i), []).map((i, n) => ({tuid: i.tuid, gid: i.gid || users[n].gid, message: i})).sort((a, b) => b.message.created - a.message.created)
          console.log('conversation', users, c)
          conversation.set(c)
        }))

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
  if (messagedb) {
    messagedb.update(message)
  }
  const uid = get(user_id)
  messages.update(m => m.concat(message))
  const {tuid: tu, uid: u, gid} = message
  let tuid = u == uid ? tu : u
  const { action } = message.message || {}
  if (action && uid && tuid) {
    contacts.update(c => {
      return c.filter(i => i.tuid == tuid).length == 0 ? c.concat({ tuid, gid, message }) : c
    })
  }
  const _conversation = get(conversation)
  console.log('conversation', _conversation, gid, message)
  if (gid) {
    if (_conversation.filter(i => i.gid == gid) > -1) {
      conversation.update(c => c.map(i => i.gid == gid ? ({ tuid, gid: i.gid || gid, message }) : i).sort((a,b) => b.message.created - a.message.created))
    } else {
      conversation.update(c => [{tuid: gid, gid, message}].concat(c))
    }
  } else if (tuid) {
    if (_conversation.filter(i => i.tuid == tuid) > -1) {
      conversation.update(c => c.map(i => i.tuid == tuid ? ({ tuid, message }) : i).sort((a,b) => b.message.created - a.message.created))
    } else {
      conversation.update(c => [{tuid, message}].concat(c))
    }
  }
  if (get(target_user_id)) {
    console.log('update chats', get(chats), get(target_user_id))
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

export * from './store'

