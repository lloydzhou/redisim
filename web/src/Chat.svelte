<script>
import page from 'page'
import './assets/litewebchat.min.css'
import redisim from './lib/redisim'
import { svgavatar } from './util'
import Input from './Input.svelte'
const { target_user_id, group_id, user_id, chats, send } = redisim

  $: {
    // console.log('target_user_id', $target_user_id, 'group_id', $group_id)
    if (!$target_user_id) {
      page('/')
    }
  }

  const onSend = (value) => {
    return send(value).then(message => {
      const m = document.getElementById(message.id)
      // console.log('send', message.id, m)
      if (m) {
        m.scrollIntoView()
      }
    })
  }

</script>
<div class="weui-tab">
  <div role="tablist" class="weui-navbar">
    <div class="header"><span on:click={e => [target_user_id.set(''), group_id.set(''), page('/')]}>&lt;</span><span class="title">{$target_user_id}</span></div>
  </div>
  <div class="weui-tab__panel lite-chatbox">
  {#each $chats as { message, id, uid, tuid }, i}
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
    <Input onOk={onSend} okText="发送" />
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
</style>
