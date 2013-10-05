## Node Avahi Pub

Quick and dirty Node bindings for Avahi service advertisements.

## API

Services are announced by calling the publish method:

`require('./avahi_pub').publish [opts]`

Opts are as follows (avahi_pub needs all of them to be defined):

* **name** - the advertised name of the service
* **type** - the type of the service (ex. `_http._tcp`)
* **data** - the txt data for the service

## Useage

See example.coffee for useage.
