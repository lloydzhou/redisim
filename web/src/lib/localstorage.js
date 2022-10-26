import { writable, derived, get } from './store';

// messages 做一个store，这个只会不断的累加不会减少
// 衍生出来last_message_id, message_count, 还有针对每个人的列表等信息
// 联系人也从这个列表衍生出来


// 使用localstorage加上内存中的数组实现
export const events = writable([])
export const messages = writable([])
export const user_id = writable('')
export const last_message = writable({})
export const last_message_id = derived([last_message], ([m]) => {
  return m.id || ''
})

try {
  let su, sm
  if (su = localStorage.getItem('user_id')) {
    sm = JSON.parse(localStorage.getItem(su))
    if (sm && sm.length) {
      last_message.set(sm[0])
      messages.set(sm)
    }
    user_id.set(su)
  }
} catch (e) {}
console.log('init', get(user_id), get(messages))
last_message.subscribe(message => {
  messages.update(m => m.concat(message))
})
messages.subscribe((m) => {
  const uid = get(user_id)
  if (uid) {
    localStorage.setItem(uid, JSON.stringify(m))
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
          res.push({ tuid: tu, gid: tu, message: i })
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

