
class Storage {
  getItem(name) {
    return localStorage.getItem()
  }
}

class RedisIM {
  constructor(option={}) {
    this.option = {
      url: '/ws',
      ...option,
    }
    this.connect()
  }
  connect() {
    this.ws = new WebSocket(this.option.base);
    this.ws.onopen = function(evt) {
      console.log("Connection open ...");
      this.emit('open', evt)
    }
    this.ws.onclose = function(evt) {
      console.log("Connection close ...", evt);
      // this.emit('close', evt)
      this.connect()
    }
    this.ws.onmessage = function(evt) {
      console.log("message ...", evt);
      if(typeof evt.data === String) {
        console.log("Received data string");
        try{
          const message = JSON.parse(evt.data)
          this.emit('message', message)
        } catch(e) {
          console.error(e)
        }
      }
    }
  }
  on(name, fn) {
    this.events[name] = [fn].concat( this.events[name] || [] );
    return () => {
      this.events[name] = this.events[name].filter(l => l !== fn);
    }
  }
  off(name) {
    delete this.events[name]
  }
  emit(name, data) {
    this.events[name].forEach(l => l(data))
  }
}

export default RedisIM


