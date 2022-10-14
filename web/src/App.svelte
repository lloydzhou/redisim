<script>
  import { onMount, afterUpdate } from 'svelte';
  import a from './assets/A.jpg'
  import './assets/litewebchat.min.css'
  export let uid = ''
  let avatar = a
  let tuid = ''
  let messages = []
  let chats = []
  let contacts = []
  let ws

  $: {
    if (tuid == uid) {
      chats = messages.filter(t => t.message && t.message.tuid == tuid && t.message.uid == tuid)
    } else if (tuid) {
      chats = messages.filter(t => t.message && (t.message.tuid == tuid || t.message.uid == tuid));
    }
  }
  // $: chats = messages.filter(t => t.message && (t.message.tuid == tuid || t.message.uid == tuid));

  $: messageLength = messages.length
  $: contactLength = contacts.length

  afterUpdate(() => {
    console.log('messages', messages, chats, contacts, uid, tuid)
  })

  const connect = (uid) => {
    const w = new WebSocket(location.origin.replace('http', 'ws') + '/ws?user_id=' + uid);
    w.onopen = () => {
      //state.messages
    }
    w.onclose = () => {
      // connect()
    }
    w.onmessage = (evt) => {
      if(typeof evt.data === "string") {
        // console.log(evt, evt.data, typeof evt.data)
        try{
          const message = JSON.parse(evt.data)
          console.log('message', message)
          if (message.message && message.message.action) {
            const { uid: u, tuid: tu, action } = message.message
            console.log('action', action)
            if (action == 'link') {
              contacts = contacts.concat({
                tuid: u == uid ? tu : u,
                avatar: a,
              })
            }
          } else {
            if (message.message) {
              messages = messages.concat(message)
            }
          }
        } catch(e) {
          console.error(e)
        }
      }
    }
    ws = w
  }

  const onChange = (e) => {
    connect(uid = e.target.value)
  }

  const send = e => {
    const message = e.target.value
    e.target.value = ''
    ws.send(JSON.stringify({
      action: 'send',
      params: [uid, tuid, 'message', message]
    }))
  }

</script>

<main>
  {#if !uid}
    <div class="login"><input placeholder="input username" on:change={onChange}></div>
  {:else}
    <div class="chat-app">
      <div class="contact">
        {#each contacts as { tuid: tu, avatar }, i}
          <div class="item" on:click={e => tuid = tu}>
            <img class="headIcon radius" src={avatar} />
            <span class="name">{tu}&nbsp;</span>
          </div>
        {/each}
      </div>
      <div class="chatbox">
        <div class="lite-chatbox">
        {#each chats.reverse() as { message }, i}
          <div class="{message.tuid == uid ? 'cleft' : 'cright'} cmsg">
            <img class="headIcon radius" src={avatar}>
            <span class="name">{message.uid}&nbsp;</span>
            <span class="content">{message.message}</span>
          </div>
        {/each}
        </div>
        <input class="message" placeholder="input message" on:change={send} />
      </div>
    </div>
  {/if}
</main>

<style>
  .chat-app{
    min-height: 98vh;
    display: flex;
  }
  .contact{
    width: 120px;
    height: 100%;
  }
  .chatbox{
    flex: 1;
    padding: 0 5px;
    border-left: 1px solid #c5d4c4;
    display: flex;
    flex-direction: column;
    justify-content: flex-end;
    height: 98vh;
  }
  .lite-chatbox{
    flex: 1;
    display: flex;
    flex-direction: column-reverse;
    padding-bottom: 10px;
  }
  .message{
    bottom: 0;
    width: 100%;
    height: 40px;
    border: 1px solid #c5d4c4;
    border-radius: 20px;
    padding: 0 20px;
    box-sizing: border-box;
  }
  .login{
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
  }
  .contact .item{
    display: flex;
    height: 50px;
    align-items: center;
    justify-content: center;
  }
  .headIcon{
    pointer-events: none;
  }
  .contact .item .headIcon{
    width: 34px;
    height: 34px;
    border: 1px solid #c5d4c4;
    border-radius: 100%;
  }
</style>
