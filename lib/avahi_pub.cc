#include <node.h>
#include <v8.h>

extern "C" {

/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

// g++ -fpermissive test.c -o test /usr/lib/i386-linux-gnu/ -lavahi-client -lavahi-common
// g++ -fpermissive test.c -o test /usr/lib/x86_64-linux-gnu/ -lavahi-client -lavahi-common

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <avahi-client/client.h>
#include <avahi-client/publish.h>

#include <avahi-common/alternative.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>

static AvahiSimplePoll *simple_poll = NULL;
static char *name = NULL;

struct ServiceInfo {
  char* name;
  char* type;
  char* data;
  AvahiEntryGroup* group;
};

static void create_services(AvahiClient *c, ServiceInfo * userdata);
static void node_avahi_pub_poll();

static void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED ServiceInfo *userdata) {
    // assert(g == userdata->group || userdata->group == NULL);
    userdata->group = g;

    /* Called whenever the entry group state changes */

    switch (state) {
        case AVAHI_ENTRY_GROUP_ESTABLISHED :
            /* The entry group has been established successfully */
            fprintf(stderr, "Service '%s' successfully established.\n", name);
            break;

        case AVAHI_ENTRY_GROUP_COLLISION : {
            char *n;

            /* A service name collision with a remote service
             * happened. Let's pick a new name */
            n = avahi_alternative_service_name(name);
            avahi_free(name);
            name = n;

            fprintf(stderr, "Service name collision, renaming service to '%s'\n", name);

            /* And recreate the services */
            create_services(avahi_entry_group_get_client(g), userdata);
            break;
        }

        case AVAHI_ENTRY_GROUP_FAILURE :

            fprintf(stderr, "Entry group failure: %s\n", avahi_strerror(avahi_client_errno(avahi_entry_group_get_client(g))));

            /* Some kind of failure happened while we were registering our services */
            avahi_simple_poll_quit(simple_poll);
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            ;
    }
}

static void create_services(AvahiClient *c, ServiceInfo * userdata) {
    char *n;
    int ret;
    assert(c);

    /* If this is the first time we're called, let's create a new
     * entry group if necessary */
    if (!(userdata->group))
    {
        if (!((userdata->group) = avahi_entry_group_new(c, entry_group_callback, userdata))) {
          fprintf(stderr, "wut1.2\n");
            fprintf(stderr, "avahi_entry_group_new() failed: %s\n", 
              avahi_strerror(avahi_client_errno(c)));
            goto fail;
        }
    }

    /* If the group is empty (either because it was just created, or
     * because it was reset previously, add our entries.  */
    if (avahi_entry_group_is_empty(userdata->group)) {
        fprintf(stderr, "Adding service '%s'\n", name);

        /* Only services with the same name should be put in the same entry 
         * group. */

        /* Add the service */
        ret = avahi_entry_group_add_service(
          userdata->group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, name, userdata->type,
          NULL, NULL, 651, userdata->data, NULL
        );
        if (ret < 0) {
            if (ret == AVAHI_ERR_COLLISION)
                goto collision;

            fprintf(stderr, "Failed to add _ipp._tcp service: %s\n", avahi_strerror(ret));
            goto fail;
        }

        /* Tell the server to register the service */
        if ((ret = avahi_entry_group_commit(userdata->group)) < 0) {
            fprintf(stderr, "Failed to commit entry group: %s\n", avahi_strerror(ret));
            goto fail;
        }

    }

    return;

collision:

    /* A service name collision with a local service happened. Let's
     * pick a new name */
    n = avahi_alternative_service_name(name);
    avahi_free(name);
    name = n;

    fprintf(stderr, "Service name collision, renaming service to '%s'\n", name);

    avahi_entry_group_reset(userdata->group);

    create_services(c, userdata);
    return;

fail:
    avahi_simple_poll_quit(simple_poll);
}

static void client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED ServiceInfo * userdata) {
    assert(c);

    fprintf(stderr, "\nwhat the fuck?\n");
    /* Called whenever the client or server state changes */

    switch (state) {
        case AVAHI_CLIENT_S_RUNNING:
            fprintf(stderr, "\nwhat the fuck?\n");

            /* The server has startup successfully and registered its host
             * name on the network, so it's time to create our services */
            create_services(c, userdata);
            break;

        case AVAHI_CLIENT_FAILURE:

            fprintf(stderr, "Client failure: %s\n", avahi_strerror(avahi_client_errno(c)));
            avahi_simple_poll_quit(simple_poll);

            break;

        case AVAHI_CLIENT_S_COLLISION:

            /* Let's drop our registered services. When the server is back
             * in AVAHI_SERVER_RUNNING state we will register them
             * again with the new host name. */

        case AVAHI_CLIENT_S_REGISTERING:

            /* The server records are now being established. This
             * might be caused by a host name change. We need to wait
             * for our own records to register until the host name is
             * properly esatblished. */

            if (userdata->group)
                avahi_entry_group_reset(userdata->group);

            break;

        case AVAHI_CLIENT_CONNECTING:
            ;
    }
}

void node_avahi_pub_publish(struct ServiceInfo* serviceInfo) {

    AvahiClient *client = NULL;
    int error;
    int ret = 1;
    struct timeval tv;
    int i;

    name = avahi_strdup(serviceInfo->name);

    /* Allocate a new client */
    client = avahi_client_new(avahi_simple_poll_get(simple_poll), 0, client_callback, serviceInfo, &error);

    /* Check wether creating the client object succeeded */
    if (!client) {
        fprintf(stderr, "Failed to create client: %s\n", avahi_strerror(error));
        avahi_free(name);
        return 1;
    }

    return 0;
}

static void node_avahi_pub_poll() {
  assert(simple_poll);
  avahi_simple_poll_iterate(simple_poll, 0);
}

static int node_avahi_pub_init() {
    /* Allocate main loop object */
    if (!(simple_poll) and !(simple_poll = avahi_simple_poll_new())) {
        fprintf(stderr, "Failed to create simple poll object.\n");
        return 1;
    }
    return 0;
}

} // End extern

using namespace v8;

Handle<Value> Publish(const Arguments& args) {
  HandleScope scope;
  Local<Object> opts = args[0]->ToObject();
  v8::Local<v8::Object> ret = v8::Object::New();
  v8::String::Utf8Value name(
    v8::Handle<v8::String>::Cast( opts->Get(String::NewSymbol("name")) )
  );
  v8::String::Utf8Value type(
    v8::Handle<v8::String>::Cast( opts->Get(String::NewSymbol("type")) )
  );
  v8::String::Utf8Value data(
    v8::Handle<v8::String>::Cast( opts->Get(String::NewSymbol("data")) )
  );

  struct ServiceInfo serviceInfo;
  serviceInfo.name = *name;
  serviceInfo.type = *type;
  serviceInfo.data = *data;
  serviceInfo.group = NULL;

  node_avahi_pub_publish(&serviceInfo);

  // ret = v8::Object::New();
  ret->Set( String::NewSymbol("name"), String::New(*name) );
  ret->Set( String::NewSymbol("type"), String::New(*type) );
  ret->Set( String::NewSymbol("data"), String::New(*data) );
  ret->Set( String::NewSymbol("_serviceInfo"), External::New(&serviceInfo) );
  return scope.Close(ret);
}

Handle<Value> Init(const Arguments& args) {
  HandleScope scope;
  node_avahi_pub_init();
  return scope.Close(Undefined());
}

Handle<Value> Poll(const Arguments& args) {
  HandleScope scope;
  node_avahi_pub_poll();
  return scope.Close(Undefined());
}

Handle<Value> Remove(const Arguments& args) {
  HandleScope scope;
  return scope.Close(Undefined());
}

void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("publish"),
      FunctionTemplate::New(Publish)->GetFunction());
  exports->Set(String::NewSymbol("poll"),
      FunctionTemplate::New(Poll)->GetFunction());
  exports->Set(String::NewSymbol("remove"),
      FunctionTemplate::New(Remove)->GetFunction());
  exports->Set(String::NewSymbol("init"),
      FunctionTemplate::New(Init)->GetFunction());
}

NODE_MODULE(avahi_pub, init)
