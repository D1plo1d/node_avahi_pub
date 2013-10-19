bindings = require('bindings')('avahi_pub.node')
os = require('os')

module.exports =
  publish: (opts) ->
    service = bindings.publish opts
    bindings.poll()
    return service
  isSupported: ->
    os.platform() == 'linux'
  kill: ->
    clearInterval interval

bindings.init()

if module.exports.isSupported()
  interval = setInterval( bindings.poll, 1000)
