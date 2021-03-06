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
#ifndef cgrpc_h
#define cgrpc_h

#import <stdlib.h>

// This file lists C functions and types used to build Swift gRPC support

#ifndef cgrpc_internal_h
// all domain types are opaque pointers
typedef void cgrpc_byte_buffer;
typedef void cgrpc_call;
typedef void cgrpc_channel;
typedef void cgrpc_completion_queue;
typedef void cgrpc_handler;
typedef void cgrpc_metadata;
typedef void cgrpc_metadata_array;
typedef void cgrpc_mutex;
typedef void cgrpc_observer;
typedef void cgrpc_observer_send_initial_metadata;
typedef void cgrpc_observer_send_message;
typedef void cgrpc_observer_send_close_from_client;
typedef void cgrpc_observer_send_status_from_server;
typedef void cgrpc_observer_recv_initial_metadata;
typedef void cgrpc_observer_recv_message;
typedef void cgrpc_observer_recv_status_on_client;
typedef void cgrpc_observer_recv_close_on_server;
typedef void cgrpc_operations;
typedef void cgrpc_server;

/** Result of a grpc call. If the caller satisfies the prerequisites of a
 particular operation, the grpc_call_error returned will be GRPC_CALL_OK.
 Receiving any other value listed here is an indication of a bug in the
 caller. */
typedef enum grpc_call_error {
  /** everything went ok */
  GRPC_CALL_OK = 0,
  /** something failed, we don't know what */
  GRPC_CALL_ERROR,
  /** this method is not available on the server */
  GRPC_CALL_ERROR_NOT_ON_SERVER,
  /** this method is not available on the client */
  GRPC_CALL_ERROR_NOT_ON_CLIENT,
  /** this method must be called before server_accept */
  GRPC_CALL_ERROR_ALREADY_ACCEPTED,
  /** this method must be called before invoke */
  GRPC_CALL_ERROR_ALREADY_INVOKED,
  /** this method must be called after invoke */
  GRPC_CALL_ERROR_NOT_INVOKED,
  /** this call is already finished
   (writes_done or write_status has already been called) */
  GRPC_CALL_ERROR_ALREADY_FINISHED,
  /** there is already an outstanding read/write operation on the call */
  GRPC_CALL_ERROR_TOO_MANY_OPERATIONS,
  /** the flags value was illegal for this call */
  GRPC_CALL_ERROR_INVALID_FLAGS,
  /** invalid metadata was passed to this call */
  GRPC_CALL_ERROR_INVALID_METADATA,
  /** invalid message was passed to this call */
  GRPC_CALL_ERROR_INVALID_MESSAGE,
  /** completion queue for notification has not been registered with the
   server */
  GRPC_CALL_ERROR_NOT_SERVER_COMPLETION_QUEUE,
  /** this batch of operations leads to more operations than allowed */
  GRPC_CALL_ERROR_BATCH_TOO_BIG,
  /** payload type requested is not the type registered */
  GRPC_CALL_ERROR_PAYLOAD_TYPE_MISMATCH
} grpc_call_error;

/** The type of completion (for grpc_event) */
typedef enum grpc_completion_type {
  /** Shutting down */
  GRPC_QUEUE_SHUTDOWN,
  /** No event before timeout */
  GRPC_QUEUE_TIMEOUT,
  /** Operation completion */
  GRPC_OP_COMPLETE
} grpc_completion_type;

typedef struct grpc_event {
  /** The type of the completion. */
  grpc_completion_type type;
  /** non-zero if the operation was successful, 0 upon failure.
   Only GRPC_OP_COMPLETE can succeed or fail. */
  int success;
  /** The tag passed to grpc_call_start_batch etc to start this operation.
   Only GRPC_OP_COMPLETE has a tag. */
  void *tag;
} grpc_event;

#endif

// directly expose a few grpc library functions
void grpc_init();
void grpc_shutdown();
const char *grpc_version_string();

// channel support
cgrpc_channel *cgrpc_channel_create(const char *address);
cgrpc_channel *cgrpc_channel_create_secure(const char *address,
                                           const char *pem_root_certs,
                                           const char *host);

void cgrpc_channel_destroy(cgrpc_channel *channel);
cgrpc_call *cgrpc_channel_create_call(cgrpc_channel *channel,
                                      const char *method,
                                      const char *host,
                                      double timeout);
cgrpc_completion_queue *cgrpc_channel_completion_queue(cgrpc_channel *channel);

// server support
cgrpc_server *cgrpc_server_create(const char *address);
cgrpc_server *cgrpc_server_create_secure(const char *address,
                                         const char *private_key,
                                         const char *cert_chain);
void cgrpc_server_stop(cgrpc_server *server);
void cgrpc_server_destroy(cgrpc_server *s);
void cgrpc_server_start(cgrpc_server *s);
cgrpc_completion_queue *cgrpc_server_get_completion_queue(cgrpc_server *s);

// completion queues
grpc_event cgrpc_completion_queue_get_next_event(cgrpc_completion_queue *cq,
                                                 double timeout);
void cgrpc_completion_queue_drain(cgrpc_completion_queue *cq);
void cgrpc_completion_queue_shutdown(cgrpc_completion_queue *cq);

// server request handlers
cgrpc_handler *cgrpc_handler_create_with_server(cgrpc_server *server);
void cgrpc_handler_destroy(cgrpc_handler *h);
cgrpc_call *cgrpc_handler_get_call(cgrpc_handler *h);
cgrpc_completion_queue *cgrpc_handler_get_completion_queue(cgrpc_handler *h);

grpc_call_error cgrpc_handler_request_call(cgrpc_handler *h,
                                           cgrpc_metadata_array *metadata,
                                           long tag);
const char *cgrpc_handler_host(cgrpc_handler *h);
const char *cgrpc_handler_method(cgrpc_handler *h);
const char *cgrpc_handler_call_peer(cgrpc_handler *h);

// call support
void cgrpc_call_destroy(cgrpc_call *call);
grpc_call_error cgrpc_call_perform(cgrpc_call *call, cgrpc_operations *operations, int64_t tag);

// operations
cgrpc_operations *cgrpc_operations_create();
void cgrpc_operations_destroy(cgrpc_operations *operations);
void cgrpc_operations_reserve_space_for_operations(cgrpc_operations *call, int max_operations);
void cgrpc_operations_add_operation(cgrpc_operations *call, cgrpc_observer *observer);

// metadata support
cgrpc_metadata_array *cgrpc_metadata_array_create();
void cgrpc_metadata_array_destroy(cgrpc_metadata_array *array);
size_t cgrpc_metadata_array_get_count(cgrpc_metadata_array *array);
const char *cgrpc_metadata_array_get_key_at_index(cgrpc_metadata_array *array, size_t index);
const char *cgrpc_metadata_array_get_value_at_index(cgrpc_metadata_array *array, size_t index);
void cgrpc_metadata_array_move_metadata(cgrpc_metadata_array *dest, cgrpc_metadata_array *src);
void cgrpc_metadata_array_append_metadata(cgrpc_metadata_array *metadata, const char *key, const char *value);

// mutex support
cgrpc_mutex *cgrpc_mutex_create();
void cgrpc_mutex_destroy(cgrpc_mutex *mu);
void cgrpc_mutex_lock(cgrpc_mutex *mu);
void cgrpc_mutex_unlock(cgrpc_mutex *mu);

// byte buffer support
void cgrpc_byte_buffer_destroy(cgrpc_byte_buffer *bb);
cgrpc_byte_buffer *cgrpc_byte_buffer_create_by_copying_data(const void *source, size_t len);
const void *cgrpc_byte_buffer_copy_data(cgrpc_byte_buffer *bb, size_t *length);

// event support
int64_t cgrpc_event_tag(grpc_event ev);

// observers

// constructors
cgrpc_observer_send_initial_metadata   *cgrpc_observer_create_send_initial_metadata(cgrpc_metadata_array *metadata);
cgrpc_observer_send_message            *cgrpc_observer_create_send_message();
cgrpc_observer_send_close_from_client  *cgrpc_observer_create_send_close_from_client();
cgrpc_observer_send_status_from_server *cgrpc_observer_create_send_status_from_server(cgrpc_metadata_array *metadata);
cgrpc_observer_recv_initial_metadata   *cgrpc_observer_create_recv_initial_metadata();
cgrpc_observer_recv_message            *cgrpc_observer_create_recv_message();
cgrpc_observer_recv_status_on_client   *cgrpc_observer_create_recv_status_on_client();
cgrpc_observer_recv_close_on_server    *cgrpc_observer_create_recv_close_on_server();

// destructor
void cgrpc_observer_destroy(cgrpc_observer *observer);

// GRPC_OP_SEND_INITIAL_METADATA


// GRPC_OP_SEND_MESSAGE
void cgrpc_observer_send_message_set_message(cgrpc_observer_send_message *observer,
                                             cgrpc_byte_buffer *message);

// GRPC_OP_SEND_CLOSE_FROM_CLIENT
// -- no special handlers --

// GRPC_OP_SEND_STATUS_FROM_SERVER
void cgrpc_observer_send_status_from_server_set_status
(cgrpc_observer_send_status_from_server *observer,
 int status);

void cgrpc_observer_send_status_from_server_set_status_details
(cgrpc_observer_send_status_from_server *observer,
 const char *statusDetails);

// GRPC_OP_RECV_INITIAL_METADATA
cgrpc_metadata_array *cgrpc_observer_recv_initial_metadata_get_metadata
(cgrpc_observer_recv_initial_metadata *observer);

// GRPC_OP_RECV_MESSAGE
cgrpc_byte_buffer *cgrpc_observer_recv_message_get_message
(cgrpc_observer_recv_message *observer);

// GRPC_OP_RECV_STATUS_ON_CLIENT
cgrpc_metadata_array *cgrpc_observer_recv_status_on_client_get_metadata
(cgrpc_observer_recv_status_on_client *observer);

long cgrpc_observer_recv_status_on_client_get_status
(cgrpc_observer_recv_status_on_client *observer);

const char *cgrpc_observer_recv_status_on_client_get_status_details
(cgrpc_observer_recv_status_on_client *observer);

// GRPC_OP_RECV_CLOSE_ON_SERVER
int cgrpc_observer_recv_close_on_server_was_cancelled
(cgrpc_observer_recv_close_on_server *observer);

#endif
