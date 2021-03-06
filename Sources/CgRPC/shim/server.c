/*
 *
 * Copyright 2016, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "internal.h"
#include "cgrpc.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

cgrpc_server *cgrpc_server_create(const char *address) {
  cgrpc_server *server = (cgrpc_server *) malloc(sizeof (cgrpc_server));
  server->server = grpc_server_create(NULL, NULL);
  server->completion_queue = grpc_completion_queue_create(NULL);
  grpc_server_register_completion_queue(server->server, server->completion_queue, NULL);
  // prepare the server to listen
  server->port = grpc_server_add_insecure_http2_port(server->server, address);
  return server;
}

cgrpc_server *cgrpc_server_create_secure(const char *address,
                                         const char *private_key,
                                         const char *cert_chain) {
  cgrpc_server *server = (cgrpc_server *) malloc(sizeof (cgrpc_server));
  server->server = grpc_server_create(NULL, NULL);
  server->completion_queue = grpc_completion_queue_create(NULL);
  grpc_server_register_completion_queue(server->server, server->completion_queue, NULL);

  grpc_ssl_pem_key_cert_pair server_credentials;
  server_credentials.private_key = private_key;
  server_credentials.cert_chain = cert_chain;

  grpc_server_credentials *credentials = grpc_ssl_server_credentials_create
  (NULL,
   &server_credentials,
   1,
   0,
   NULL);
  
  // prepare the server to listen
  server->port = grpc_server_add_secure_http2_port(server->server, address, credentials);
  return server;
}

void cgrpc_server_stop(cgrpc_server *server) {
  grpc_server_shutdown_and_notify(server->server,
                                  server->completion_queue,
                                  cgrpc_create_tag(0));
}

void cgrpc_server_destroy(cgrpc_server *server) {
  grpc_server_shutdown_and_notify(server->server,
                                  server->completion_queue,
                                  cgrpc_create_tag(1000));
  grpc_event completion_event =
  grpc_completion_queue_pluck(server->completion_queue,
                              cgrpc_create_tag(1000),
                              cgrpc_deadline_in_seconds_from_now(5),
                              NULL);
  assert(completion_event.type == GRPC_OP_COMPLETE);
  grpc_server_destroy(server->server);
  server->server = NULL;

  grpc_completion_queue_shutdown(server->completion_queue);
  cgrpc_completion_queue_drain(server->completion_queue);
  grpc_completion_queue_destroy(server->completion_queue);
}

void cgrpc_server_start(cgrpc_server *server) {
  grpc_server_start(server->server);
}

cgrpc_completion_queue *cgrpc_server_get_completion_queue(cgrpc_server *s) {
  return s->completion_queue;
}
