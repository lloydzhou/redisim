<script>
  import 'weui/src/style/widget/weui-media-box/weui-media-box.less'
  import redisim from './lib/redisim'
  import { svgavatar } from './util'
  const { user_id } = redisim
</script>
<div>
<a href="javascript:" class="weui-media-box weui-media-box_appmsg">
  <div aria-hidden="true" class="weui-media-box__hd">
    <img class="weui-media-box__thumb" src={svgavatar($user_id)} alt="">
  </div>
  <div aria-hidden="true" id="js_p1m1_bd" class="weui-media-box__bd">
      <strong class="weui-media-box__title">{$user_id}</strong>
      <p class="weui-media-box__desc">{$user_id}</p>
  </div>
</a>
</div>
