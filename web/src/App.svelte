<script>
  import { get } from 'svelte/store';
  import { onMount, afterUpdate, tick } from 'svelte';
  import a from './assets/A.jpg'
  import './assets/litewebchat.min.css'
  // import { messages, contacts, target_user_id, user_id, chats, last_message_id } from './redisim'
  import redisim from './redisim'
  const { connect, messages, contacts, target_user_id, group_id, user_id, chats, last_message_id, send, link, join } = redisim
  export let uid = ''
  let avatar = a

  const hashCode = s => s.split('').reduce((a,b)=>{a=((a<<5)-a)+b.charCodeAt(0);return a&a},0)
  const colors = ['#ff0000', '#00FF00', '#0000FF']
  const color = (name) => {
    return colors[hashCode(name) % colors.length]
  }

  const svgavatar = (name) => 'data:image/svg+xml;utf8,' + encodeURIComponent(`<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="64px" height="64px" viewBox="0 0 64 64" version="1.1"><circle fill="${color(name)}" width="64" height="64" cx="32" cy="32" r="32"/><text x="50%" y="50%" style="color: #ffffff;line-height: 1;font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Oxygen', 'Ubuntu', 'Fira Sans', 'Droid Sans', 'Helvetica Neue', sans-serif;" alignment-baseline="middle" text-anchor="middle" font-size="26" font-weight="400" dy=".1em" dominant-baseline="middle" fill="#ffffff">${name.slice(0, 2).toUpperCase()}</text></svg>`)

  $: {
    user_id.set(uid)
  }

  afterUpdate(() => {
    // console.log('messages', $messages, 'contacts', $contacts, 'chats', $chats, 'last_message_id', $last_message_id, 'target_user_id', target_user_id)
  })

  const onChange = (e) => {
    connect(e.target.value)
  }

  const onSend = e => {
    const message = e.target.value
    e.target.value = ''
    send(message).then(message => {
      tick().then(() => {
        const m = document.getElementById(message.id)
        // console.log('send', message.id, m)
        if (m) {
          m.scrollIntoView()
        }
      })
    })
  }

  const onLink = e => {
    const tuid = e.target.value
    e.target.value = ''
    link(tuid)
  }

  const onJoin = e => {
    const gid = e.target.value
    e.target.value = ''
    join(gid)
  }

</script>

<main>
  {#if !$user_id}
    <div class="login"><input class="login-name" placeholder="input username" on:change={onChange}></div>
  {:else}
    <div class="chat-app">
      <div class="contact">
        <div class="link"><input class="link-name" placeholder="link user" on:change={onLink}></div>
        <div class="link"><input class="link-name" placeholder="join group" on:change={onJoin}></div>
        {#each $contacts as { tuid: tu, gid, avatar = a }, i}
          <div class="item" on:click={e => [target_user_id.set(tu), group_id.set(gid)]}>
            <img class="headIcon radius" src={svgavatar(tu)} alt="" />
            <span class="name">{tu}&nbsp;</span>
          </div>
        {/each}
      </div>
      <div class="chatbox">
        <div class="lite-chatbox">
        {#each $chats.reverse() as { message, id, uid, tuid }, i}
          {#if message.action == 'link'}
            <div class="tips" id={id}>
              <span class="tips-success">{uid == $user_id ? '您' : uid}已成功添加{tuid == $user_id ? '您' : tuid}</span>
            </div>
          {:else if message.action == 'join'}
            <div class="tips" id={id}>
              <span class="tips-success">{uid == $user_id ? '您' : uid}加入群聊</span>
            </div>
          {:else}
            <div class="{uid == $user_id ? 'cright' : 'cleft'} cmsg" id={id}>
              <img class="headIcon radius" src={svgavatar(uid)} alt="" />
              <span class="name">{uid}&nbsp;</span>
              <span class="content">{message.message}</span>
            </div>
          {/if}
        {/each}
        </div>
        {#if $target_user_id}
          <input class="message" placeholder="input message" on:change={onSend} />
        {/if}
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
    max-width: 240px;
    width: 20%;
    height: 100%;
    padding: 0 10px;
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
  .login-name {
    height: 40px;
    border: 1px solid #c5d4c4;
    border-radius: 20px;
    padding: 0 20px;
  }
  .link-name {
    height: 40px;
    border: 1px solid #c5d4c4;
    border-radius: 20px;
    padding: 0 20px;
    box-sizing: border-box;
    width: 100%;
  }
  .contact .item{
    display: flex;
    height: 50px;
    align-items: center;
    justify-content: space-between;
  }
  .headIcon{
    pointer-events: none;
  }
  .contact .item .headIcon{
    width: 34px;
    height: 34px;
    border: 1px solid #c5d4c4;
    border-radius: 100%;
    margin-right: 6px;
  }
  .contact .item .name{
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    flex: 1;
  }
</style>
