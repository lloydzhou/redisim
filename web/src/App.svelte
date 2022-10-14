<script>
  import { get } from 'svelte/store';
  import { onMount, afterUpdate } from 'svelte';
  import a from './assets/A.jpg'
  import './assets/litewebchat.min.css'
  // import { messages, contacts, target_user_id, user_id, chats, last_message_id } from './redisim'
  import redisim from './redisim'
  const { connect, messages, contacts, target_user_id, group_id, user_id, chats, last_message_id, send, link, join } = redisim
  export let uid = ''
  let avatar = a

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
    send(message)
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
            <img class="headIcon radius" src={avatar} />
            <span class="name">{tu}&nbsp;</span>
          </div>
        {/each}
      </div>
      <div class="chatbox">
        <div class="lite-chatbox">
        {#each $chats.reverse() as { message, id }, i}
          {#if message.action == 'link'}
            <div class="tips">
              <span class="tips-success">{message.uid == $user_id ? '您' : message.uid}已成功添加{message.tuid == $user_id ? '您' : message.tuid}</span>
            </div>
          {:else if message.action == 'join'}
            <div class="tips">
              <span class="tips-success">{message.uid == $user_id ? '您' : message.uid}加入群聊</span>
            </div>
          {:else}
            <div class="{message.uid == $user_id ? 'cright' : 'cleft'} cmsg" id={id}>
              <img class="headIcon radius" src={avatar} />
              <span class="name">{message.uid}&nbsp;</span>
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
