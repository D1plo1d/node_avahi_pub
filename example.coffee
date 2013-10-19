avahi = require('./avahi_pub')

# Validate that avahi is supported on this platform (avahi is linux-only)
if avahi.isSupported()
  console.log "Node Avahi Pub is supported"

  # Advertise a _construct._tcp service named MyService with txtdata on port 1337
  service = avahi.publish
    name: "MyService"
    type: "_construct._tcp"
    data: "myData"
    port: 1337

  service2 = avahi.publish
    name: "Another Freaking Service"
    type: "_stuff._tcp"
    data: "myData"
    port: 5555

  # Stop advertising that service 4 seconds later
  remove = ->
    service.remove()
    service2.remove()
    avahi.kill()
    console.log "all done!"
  setTimeout remove, 8000

else
  console.log "Node Avahi Pub is not supported"