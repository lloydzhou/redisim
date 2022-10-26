<script>
import { onMount } from 'svelte'
import 'weui/src/style/widget/weui-tab/weui-tab.less'
import page from 'page'
import Address from './Address.svelte'
import Mine from './Mine.svelte'
import ChatList from './ChatList.svelte'
import Chat from './Chat.svelte'
import Login from './Login.svelte'
import { chatsvg, addresssvg, faxiansvg, minesvg } from './util'

import redisim from './lib/redisim'

const { user_id, unread } = redisim

let currentComponent = ChatList
let params
let active

const routes = [
  {
    path: '/',
    component: ChatList,
    navbar: 'chat',
  },
  {
    path: '/address',
    component: Address,
    navbar: 'address',
  },
  {
    path: '/faxian',
    component: ChatList,
    navbar: 'faxian',
  },
  {
    path: '/mine',
    component: Mine,
    navbar: 'mine',
  },
  {
    path: '/chat/:id',
    component: Chat,
  },
  {
    path: '/login',
    component: Login,
  },
  {
    path: '/*',
    component: ChatList,
    navbar: 'chat',
  },
]

routes.forEach(({path, component, navbar}) => {
  page(path, (ctx, next) => {
    params = ctx.params
    next()
  }, (ctx) => {
    // console.log('user_id', $user_id, ctx)
    active = navbar
    if (!$user_id && ctx.path != '/login') {
      page.redirect("/login");
    } else {
      currentComponent = component
    }
  })
})


onMount(() => page.start())

</script>
<div class="page">
  <div class="page__bd" style="height: 100%;">
    {#if active}
      <div class="weui-tab">
        <div class="weui-tab__panel">
          <svelte:component this={currentComponent} />
        </div>
        <div role="tablist" aria-label="选项卡标题" class="weui-tabbar">
          <div role="tab" class="weui-tabbar__item" class:weui-bar__item_on={active == 'chat'} on:click={e => [active = 'chat', page('/')]}>
            <div style="display: inline-block; position: relative;">
              <img src={chatsvg(active == 'chat' ? '#07c160' : '#00000080')} alt="" class="weui-tabbar__icon">
              {#if $unread}
                <span class="weui-badge" style="position: absolute; top: -2px; right: -13px;">8</span>
              {/if}
            </div>
            <p class="weui-tabbar__label">消息</p>
          </div>
          <div role="tab" class="weui-tabbar__item" class:weui-bar__item_on={active == 'address'} on:click={e => [active = 'address', page('/address')]}>
            <div style="display: inline-block; position: relative;">
              <img src={addresssvg(active == 'address' ? '#07c160' : '#00000080')} alt="" class="weui-tabbar__icon">
            </div>
            <p class="weui-tabbar__label">好友</p>
          </div>
          <div role="tab" class="weui-tabbar__item" class:weui-bar__item_on={active == 'faxian'} on:click={e => [active = 'faxian', page('/faxian')]}>
            <div style="display: inline-block; position: relative;">
              <img src={faxiansvg(active == 'faxian' ? '#07c160' : '#00000080')} alt="" class="weui-tabbar__icon">
            </div>
            <p class="weui-tabbar__label">发现</p>
          </div>
          <div role="tab" class="weui-tabbar__item" class:weui-bar__item_on={active == 'mine'} on:click={e => [active = 'mine', page('/mine')]}>
            <div style="display: inline-block; position: relative;">
              <img src={minesvg(active == 'mine' ? '#07c160' : '#00000080')} alt="" class="weui-tabbar__icon">
            </div>
            <p class="weui-tabbar__label">我的</p>
          </div>
        </div>
      </div>
    {:else}
      <svelte:component this={currentComponent} />
    {/if}
  </div>
</div>
<style>
</style>
