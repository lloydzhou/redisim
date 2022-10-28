<script>
  import page from 'page'
  import redisim from './lib/redisim'
  import { svgavatar, showTime } from './util'
  import Input from './Input.svelte'
  const { contacts, target_user_id, group_id, link, join } = redisim
</script>
<Input onOk={tuid => link(tuid)} okText="添加" mode="navbar" placeholder="link user" />
<Input onOk={gid => join(gid)} okText="加入" mode="navbar" placeholder="join group" />
{#each $contacts as { tuid: tu, gid, message }, i}
  <div class="item" on:click={e => [target_user_id.set(tu), group_id.set(gid), page('/chat/' + tu)]}>
    <img class="headIcon radius" src={svgavatar(tu)} alt="" />
    <div class="info">
      <div class="name">{tu}&nbsp;</div>
      <span>{showTime(message.id)}</span>
    </div>
  </div>
{/each}
<style>
  .link-name {
    height: 40px;
    border: 1px solid #c5d4c4;
    border-radius: 20px;
    padding: 0 20px;
    box-sizing: border-box;
    width: 100%;
    margin: 0 auto;
  }
  .item{
    display: flex;
    height: 50px;
    align-items: center;
    justify-content: space-between;
  }
  .headIcon{
    pointer-events: none;
  }
  .item .headIcon{
    width: 34px;
    height: 34px;
    border: 1px solid #c5d4c4;
    border-radius: 100%;
    margin-right: 6px;
  }
  .item .info{
    flex: 1;
    display: flex;
    justify-content: space-between;
  }
  .item .name{
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
</style>
