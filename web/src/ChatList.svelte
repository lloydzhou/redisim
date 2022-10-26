<script>
  import page from 'page'
  import redisim from './lib/redisim'
  import { svgavatar, showTime } from './util'
  const { conversation, target_user_id, group_id } = redisim

  const summary = (message) => {
    return message.message || ''
  }

</script>
{#each $conversation as { tuid: tu, gid, message }, i}
  <div class="item" on:click={e => [target_user_id.set(tu), group_id.set(gid), page('/chat/' + tu)]}>
    <img class="headIcon radius" src={svgavatar(tu)} alt="" />
    <div class="info">
      <div class="name">{tu}&nbsp;{gid}</div>
      <div class="desc"><span>{summary(message.message)}&nbsp;</span><span>{showTime(message.id)}</span></div>
    </div>
  </div>
{/each}
<style>
  .item{
    display: flex;
    height: 50px;
    align-items: center;
    justify-content: space-between;
    padding: 0 5px;
    box-sizing: border-box;
    border-bottom: 1px solid #c5d4c466;
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
  }
  .item .name, .item{
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
  .desc {
    font-size: 10px;
    color: #666;
    display: flex;
    justify-content: space-between;
  }
</style>
