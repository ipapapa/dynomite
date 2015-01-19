/*
 * Dynomite - A thin, distributed replication layer for multi non-distributed storages.
 * Copyright (C) 2014 Netflix, Inc.
 */ 

/*
 * twemproxy - A fast and lightweight proxy for memcached protocol.
 * Copyright (C) 2011 Twitter, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dyn_core.h"
#include "dyn_token.h"


#ifndef _DYN_SERVER_H_
#define _DYN_SERVER_H_



/*
 * server_pool is a collection of servers and their continuum. Each
 * server_pool is the owner of a single proxy connection and one or
 * more client connections. server_pool itself is owned by the current
 * context.
 *
 * Each server is the owner of one or more server connections. server
 * itself is owned by the server_pool.
 *
 *  +-------------+
 *  |             |<---------------------+
 *  |             |<------------+        |
 *  |             |     +-------+--+-----+----+--------------+
 *  |   pool 0    |+--->|          |          |              |
 *  |             |     | server 0 | server 1 | ...     ...  |
 *  |             |     |          |          |              |--+
 *  |             |     +----------+----------+--------------+  |
 *  +-------------+                                             //
 *  |             |
 *  |             |
 *  |             |
 *  |   pool 1    |
 *  |             |
 *  |             |
 *  |             |
 *  +-------------+
 *  |             |
 *  |             |
 *  .             .
 *  .    ...      .
 *  .             .
 *  |             |
 *  |             |
 *  +-------------+
 *            |
 *            |
 *            //
 */

typedef rstatus_t (*hash_t)(const char *, size_t, struct dyn_token *);

struct continuum {
    uint32_t index;  /* dyn_peer index */
    uint32_t value;  /* hash value, used by ketama */
    struct dyn_token *token;  /* used in vnode/dyn_token situations */
};

struct rack {
    struct string      *name;
    struct string      *dc;
    uint32_t           ncontinuum;           /* # continuum points */
    uint32_t           nserver_continuum;    /* # servers - live and dead on continuum (const) */
    struct continuum   *continuum;           /* continuum */
};



struct server {
    uint32_t           idx;           /* server index */
    struct server_pool *owner;        /* owner pool */

    struct string      pname;         /* name:port:weight (ref in conf_server) */
    struct string      name;          /* name (ref in conf_server) */
    uint16_t           port;          /* port */
    uint32_t           weight;        /* weight */
    int                family;        /* socket family */
    socklen_t          addrlen;       /* socket length */
    struct sockaddr    *addr;         /* socket address (ref in conf_server) */

    uint32_t           ns_conn_q;     /* # server connection */
    struct conn_tqh    s_conn_q;      /* server connection q */

    int64_t            next_retry;    /* next retry time in usec */
    uint32_t           failure_count; /* # consecutive failures */
    
    struct string      rack;          /* logical rack */
    struct string      dc;            /* server's dc */
    struct array       tokens;        /* DHT tokens this peer owns */
    bool               is_local;      /* is this peer the current running node?  */
    unsigned           is_seed:1;     /* seed? */
    unsigned           processed:1;   /* flag to indicate whether this has been processed */
    unsigned           is_secure:1;   /* is the connection to the server secure? */
    uint8_t            state;         /* state of the server - used mainly in peers  */
};

struct server_pool {
    uint32_t           idx;                  /* pool index */
    struct context     *ctx;                 /* owner context */
    struct conf_pool   *conf_pool;           /* back reference to conf_pool */

    struct conn        *p_conn;              /* proxy connection (listener) */
    uint32_t           dn_conn_q;            /* # client connection */
    struct conn_tqh    c_conn_q;             /* client connection q */

    struct array       server;               /* server[] */
    struct array       racks;                /* racks info  */
    uint32_t           nlive_server;         /* # live server */
    int64_t            next_rebuild;         /* next distribution rebuild time in usec */

    struct string      name;                 /* pool name (ref in conf_pool) */
    struct string      addrstr;              /* pool address (ref in conf_pool) */
    uint16_t           port;                 /* port */
    int                family;               /* socket family */
    socklen_t          addrlen;              /* socket length */
    struct sockaddr    *addr;                /* socket address (ref in conf_pool) */
    int                dist_type;            /* distribution type (dist_type_t) */
    int                key_hash_type;        /* key hash type (hash_type_t) */
    hash_t             key_hash;             /* key hasher */
    struct string      hash_tag;             /* key hash tag (ref in conf_pool) */
    int                timeout;              /* timeout in msec */
    int                backlog;              /* listen backlog */
    uint32_t           client_connections;   /* maximum # client connection */
    uint32_t           server_connections;   /* maximum # server connection */
    int64_t            server_retry_timeout; /* server retry timeout in usec */
    uint32_t           server_failure_limit; /* server failure limit */
    unsigned           auto_eject_hosts:1;   /* auto_eject_hosts? */
    unsigned           preconnect:1;         /* preconnect? */
   // unsigned           redis:1;              /* redis? */
    /* dynomite */
    struct string      seed_provider;
    struct array       seeds;                /*dyn seeds */
    struct array       peers;
    struct conn        *d_conn;              /* dnode connection (listener) */
    struct string      d_addrstr;            /* pool address (ref in conf_pool) */
    uint16_t           d_port;               /* port */
    int                d_family;             /* socket family */
    socklen_t          d_addrlen;            /* socket length */
    struct sockaddr    *d_addr;              /* socket address (ref in conf_pool) */
    int                d_timeout;            /* peer timeout in msec */
    int                d_backlog;            /* listen backlog */ 
    int64_t            d_retry_timeout;      /* peer retry timeout in usec */
    uint32_t           d_failure_limit;      /* peer failure limit */
    uint32_t           d_connections;        /* maximum # dyn connections */
    struct string      rack;                   /* the rack for this node */
    struct array       tokens;               /* the DHT tokens for this server */

    int                g_interval;           /* gossip interval */
    struct string      dc;                   /* server's dc */
    struct string      env;                  /* aws, network, ect */

    /* Yannis */
    int data_store;

};

void server_ref(struct conn *conn, void *owner);
void server_unref(struct conn *conn);
int server_timeout(struct conn *conn);
bool server_active(struct conn *conn);
rstatus_t server_init(struct array *server, struct array *conf_server, struct server_pool *sp);
void server_deinit(struct array *server);
struct conn *server_conn(struct server *server);
rstatus_t server_connect(struct context *ctx, struct server *server, struct conn *conn);
void server_close(struct context *ctx, struct conn *conn);
void server_connected(struct context *ctx, struct conn *conn);
void server_ok(struct context *ctx, struct conn *conn);

//struct rack *server_get_rack(struct server_pool *pool, struct string *rackname);
struct rack *server_get_rack(struct server_pool *pool, struct string *rackname, struct string *dc);
void rack_init(struct rack *rack);
rstatus_t rack_deinit(struct rack *rack);

struct conn *server_pool_conn(struct context *ctx, struct server_pool *pool, uint8_t *key, uint32_t keylen);
rstatus_t server_pool_run(struct server_pool *pool);
rstatus_t server_pool_preconnect(struct context *ctx);
void server_pool_disconnect(struct context *ctx);
rstatus_t server_pool_init(struct array *server_pool, struct array *conf_pool, struct context *ctx);
void server_pool_deinit(struct array *server_pool);

#endif
