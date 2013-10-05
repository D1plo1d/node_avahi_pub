bindings = require('bindings')('avahi_pub.node')
os = require('os')

module.exports =
  publish: (opts) ->
    service = bindings.publish opts
    bindings.poll()
    return service
  isSupported: ->
    os.platform() == 'linux'

bindings.init()
setInterval( bindings.poll, 1000) if module.exports.isSupported()
