avahi = require('./avahi_pub')

# Advertise a _construct._tcp service named MyService with txtdata 
service = avahi.publish
  name: "MyService"
  type: "_construct._tcp"
  data: "myData"

# Stop advertising that service 4 seconds later
setTimeout( ( -> service.remove() ), 4000);
