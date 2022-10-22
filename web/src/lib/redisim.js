import {
  user_id, target_user_id, group_id,
  messages, last_message, last_message_id,
  conversation, contacts, chats,
  get,
} from './localstorage'

export function RedisIM(url) {
  let ws
  const connect = (uid) => {
    return new Promise((resolve, reject) => {
      if (get(user_id) != uid) {
        user_id.set(uid)
        messages.set([])
      }
      // 先尝试关闭
      if (ws && ws.readyState != 3) {
        ws.close()
      }
      ws = new WebSocket(url + '?user_id=' + uid + '&last_message_id=' + get(last_message_id))
      ws.onmessage = (evt) => {
        if(typeof evt.data === "string") {
          // console.log(evt, evt.data, typeof evt.data)
          try{
            const message = JSON.parse(evt.data)
            // console.log('message', message)
            if (message.id) {
              messages.update(m => m.concat(message))
            }
          } catch(e) {
            console.error(e)
          }
        }
      }
      ws.onopen = (evt) => {
        resolve(evt)
      }
      ws.onclose = (evt) => {
        reject(evt)
      }
    })
  }

  const select_user = (tuid) => target_user_id.set(tuid)

  const action = (action, params) => {
    const id = +new Date();
    let response = ''
    return new Promise((resolve, reject) => {
      const unsubscribe = messages.subscribe(messages => {
        const m = messages.slice(-1).pop()
        if (m) {
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
        }
      })
      setTimeout(unsubscribe, 3000)
      ws.send(JSON.stringify({id, action, params}))
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

  if (get(user_id)) {
    connect(get(user_id))
  }

  // 启动的时候不主动触发连接，而是在用户登录的时候触发，所以监听事件的时候，尝试判断一下
  document.addEventListener("visibilitychange", () => {
    if (!document.hidden && get(user_id)) {
      console.log('visibilitychange reconnect')
      // alert('reconnect' + document.visibilityState)
      connect(get(user_id))
    }
  });

  return {
    messages,
    contacts,
    conversation,
    chats,
    user_id,
    target_user_id,
    group_id,
    last_message_id,
    connect,
    select_user,
    send,
    link,
    join,
  }
}

export const redisim = RedisIM(location.origin.replace('http', 'ws') + '/ws')
export default redisim

