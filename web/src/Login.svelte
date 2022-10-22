<script>
import page from 'page'
import redisim from './lib/redisim'

const { user_id, connect } = redisim

$: {
  console.log('user_id', $user_id)
  if ($user_id) {
    page.replace('/')
  }
}
const onChange = (e) => {
  connect(e.target.value).then((evt) => {
    console.log(evt, page)
    // page.redirect('/')
    page('/')
  })
}

</script>
<div class="login"><input class="login-name" placeholder="input username" on:change={onChange}></div>
<style>
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
</style>
