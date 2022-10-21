<script>
import page from 'page'
import './assets/litewebchat.min.css'
import redisim from './redisim'
import { svgavatar } from './util'
const { target_user_id, group_id, user_id, chats, send } = redisim

  $: {
    console.log('target_user_id', $target_user_id)
    if (!$target_user_id) {
      page('/')
    }
  }

  let value = ''
  const onInput = (e) => {
    value = e.target.value
  }
  const onKeydown = e => {
    if (e.keyCode === 13) {
      onSend(e)
    }
  }
  const onSend = e => {
    if (!value) return;
    send(value).then(message => {
      const m = document.getElementById(message.id)
      // console.log('send', message.id, m)
      if (m) {
        m.scrollIntoView()
      }
      value = ''
    })
  }
</script>
<div class="weui-tab">
  <div role="tablist" class="weui-navbar">
    <div class="header"><span on:click={e => [target_user_id.set(''), group_id.set(''), page('/')]}>&lt;</span><span class="title">{$target_user_id}</span></div>
  </div>
  <div class="weui-tab__panel lite-chatbox">
  {#each $chats.slice(0).reverse() as { message, id, uid, tuid }, i}
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
    <div role="tablist" aria-label="输入框" class="weui-tabbar">
      <input class="message" placeholder="input message" bind:value={value} on:keydown={onKeydown} on:input={onInput} />
      {#if value}
        <button class="send" on:click={onSend}>发送</button>
      {/if}
    </div>
  {/if}
</div>
<style lang="less">
  .header{
    width: 100%;
    height: 40px;
    line-height: 40px;
    display: flex;
    padding: 0 20px;
    .title{
      flex: 1;
      text-align: center;
    }
  }
  .lite-chatbox{
    flex: 1;
    display: flex;
    flex-direction: column-reverse;
    padding-bottom: 10px;
  }
  .message{
    width: 100%;
    margin: 0 auto;
    height: 40px;
    border: 1px solid #c5d4c4;
    border-radius: 20px;
    padding: 0 20px;
    box-sizing: border-box;
    margin: 5px;
  }
  .send {
    width: 100px;
    border-radius: 20px;
    background: linear-gradient(20deg,#3f8fe1cc 0%,#44d7c9 100%);
    height: 40px;
    margin: 5px 5px 5px 10px;
    color: #ffffff;
  }
</style>
