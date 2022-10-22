// import { writable, derived, get } from 'svelte/store';

export function safe_not_equal(a, b) {
	return a != a ? b == b : a !== b || ((a && typeof a === 'object') || typeof a === 'function');
}

export function writable(value) {
  let subscribers = [];
  function set(new_value) {
    if (safe_not_equal(value, new_value)) {
      value = new_value;
      subscribers.forEach(l => l(value))
    }
  }
  function subscribe(run) {
    run(value)
    subscribers = subscribers.concat(run)
    return () => subscribers = subscribers.filter(l => l !== run)
  }
  return { set, update: (fn) => set(fn(value)), subscribe }
}

export function get(store) {
  let value
  store.subscribe(_ => value = _)()
  return value
}
export function derived(stores, fn) {
  const values = writable([])
  const unsubscribes = stores.forEach(store => {
    store.subscribe(() => {
      values.set(fn(stores.map(get)))
    })
  })
  return values
}

