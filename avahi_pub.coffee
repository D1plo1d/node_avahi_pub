bindings = require('bindings')('avahi_pub.node')

bindings.init()
setInterval( bindings.poll, 1000)

module.exports = publish: (opts) ->
  service = bindings.publish opts
  bindings.poll()
  return service
