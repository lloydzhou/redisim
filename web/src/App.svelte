<script>
import 'weui/src/style/widget/weui-tab/weui-tab.less'
import page from 'page'
import ChatList from './ChatList.svelte'
import Chat from './Chat.svelte'
import Login from './Login.svelte'

import redisim from './redisim'

const { user_id } = redisim

let current = ChatList
let params

const routes = [
  {
    path: '/',
    component: ChatList,
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
  },
]

routes.forEach(({path, component}) => {
  page(path, (ctx, next) => {
    params = ctx.params
    next()
  }, (ctx) => {
    // console.log('user_id', $user_id, ctx)
    if (!$user_id && ctx.path != '/login') {
      page.redirect("/login");
    } else {
      current = component
    }
  })
})

// start
page.start()

</script>
<div class="page">
  <div class="page__bd" style="height: 100%;">
    <svelte:component this={current} />
  </div>
</div>
<style>
</style>
