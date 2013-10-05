## Node Avahi Pub

Quick and dirty Node bindings for Avahi service advertisements.

## Why?

Node MDNS used Avahi's bonjour compatibility API which meant advertisements 
didn't work. Node Avahi Pub uses the native Avahi API directly to get around 
that limitation.

## API

### isSupported()

Returns `true` if the platform is supported by node\_avahi\_pub.

### publish( [opts] )

Services are announced by calling the publish method:

`require('./avahi_pub').publish( [opts] )`

Opts are as follows (avahi_pub needs all of them to be defined):

* **name** - the advertised name of the service
* **type** - the type of the service (ex. `_http._tcp`)
* **port** - the port number the service is accessible on
* **data** - the txt data for the service

**return**: calling publish returns a new service object

### Service Objects

`service.remove()` - stops the service's advertisement on the network.

## Useage

See example.coffee for useage.

## License

Licenced under the MIT License. See http://opensource.org/licenses/MIT