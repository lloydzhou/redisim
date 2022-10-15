import { readable, derived, get } from 'svelte/store';
import { writable } from 'svelte/store';

function createListStore() {
	const list = writable([]);
	const { subscribe, set, update } = list;
  return {
		subscribe,
		push: (i) => update(n => n.concat(i)),
		pop: () => update(n => n.slice(0, -1)),
		shift: () => update(n => n.slice(1)),
		get: (id) => update(n => n.filter(i => i.id === id).pop()),
		remove: (id) => update(n => n.filter(i => i.id !== id)),
		clear: () => set([]),
		length: () => get(list).length,
  }
}

// messages 做一个store，这个只会不断的累加不会减少
// 衍生出来last_message_id, message_count, 还有针对每个人的列表等信息
// 联系人也从这个列表衍生出来

export function RedisIM(url) {
  const messages = createListStore()
  const user_id = writable('')
  const target_user_id = writable('')
  const group_id = writable('')
  const contacts = derived([messages, user_id], ([m, uid]) => {
    const res = []
    if (uid) {
      const s = new Set()
      m.forEach(i => {
        const { tuid: tu, uid: u, action, ...p } = i.message || {}
        if (action == 'link') {
          const tuid = u == uid ? tu : u
          if (!s.has(tuid)){
            s.add(tuid)
            res.push({ tuid, ...p })
          }
        }
        else if (action == 'join') {
          if (!s.has(tu)){
            s.add(tu)
            res.push({ tuid: tu, ...p })
          }
        }
      })
    }
    return res
  })
  const chats = derived([messages, user_id, target_user_id, group_id], ([m, uid, tuid, gid]) => {
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
  const last_message = derived(messages, m => {
    for (let i=m.length-1; i >0; i--) {
      if (m[i].message) {
        return message
      }
    }
    return {id: ''}
  })
  const last_message_id = derived(last_message, m => {
    return m.id
  })
  let ws
  const connect = (uid) => {
    if (get(user_id) != uid) {
      user_id.set(uid)
      messages.clear()
    }
    ws = new WebSocket(url + '?user_id=' + uid + '&last_message_id=' + get(last_message_id))
    ws.onmessage = (evt) => {
      if(typeof evt.data === "string") {
        // console.log(evt, evt.data, typeof evt.data)
        try{
          const message = JSON.parse(evt.data)
          // console.log('message', message)
          if (message.id) {
            messages.push(message)
          }
        } catch(e) {
          console.error(e)
        }
      }
    }
  }

  const select_user = (tuid) => target_user_id.set(tuid)

  const action = (action, params) => {
    const id = +new Date();
    let response = ''
    return new Promise((resolve, reject) => {
      ws.send(JSON.stringify({id, action, params}))
      const unsubscribe = messages.subscribe(messages => {
        const m = messages.slice(-1).pop()
        if (response == m.id) {
          resolve(m)
          unsubscribe()
        }
        if (m.response && m.id === id) {
          response = m.response
          if (typeof response == "string" && response.split('-').length == 2) {
          } else {
            resolve(m.response)
            unsubscribe()
          }
        }
      })
      setTimeout(unsubscribe, 3000)
    })
  }
  const send = (message) => {
    const tuid = get(target_user_id)
    const uid = get(user_id)
    if (tuid) {
      return action(get(group_id) ? 'gsend' : 'send', [uid, tuid, 'message', message])
    }
  }
  const link = (tuid) => {
    const uid = get(user_id)
    if (tuid) {
      return action('link', [uid, tuid])
    }
  }

  const join = (gid) => {
    const uid = get(user_id)
    if (gid) {
      return action('join', [uid, gid])
    }
  }

  return {
    connect,
    select_user,
    messages,
    contacts,
    chats,
    user_id,
    target_user_id,
    group_id,
    last_message_id,
    send,
    link,
    join,
  }
}

export const redisim = RedisIM(location.origin.replace('http', 'ws') + '/ws')
export default redisim

