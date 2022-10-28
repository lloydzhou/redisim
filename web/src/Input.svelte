<script>
  export let placeholder = 'input message'
  export let mode = 'navbar'
  export let onOk
  export let okText = '确定'

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
    console.log('onSend', e, value)
    if (value) {
      onOk && onOk(value).then(() => value = '')
    }
  }
</script>
<div role="tablist" aria-label="输入框" class={mode == 'navbar' ? "weui-navbar" : "weui-tabbar"}>
  <input class="message" placeholder={placeholder} bind:value={value} on:keydown={onKeydown} on:input={onInput} />
  {#if value}
    <button class="send" on:click={onSend}>{okText}</button>
  {/if}
</div>
<style lang="less">
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

