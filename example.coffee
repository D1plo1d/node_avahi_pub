console.log avahi = require('bindings')('avahi_pub.node')

avahi.init()
setInterval( avahi.poll, 1000)

console.log avahi.publish(name: "Test", type: "_construct._tcp", data: "data")
avahi.poll()

