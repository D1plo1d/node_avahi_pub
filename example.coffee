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

  # Stop advertising that service 4 seconds later
  setTimeout( ( -> service.remove() ), 4000);

else
  console.log "Node Avahi Pub is not supported"