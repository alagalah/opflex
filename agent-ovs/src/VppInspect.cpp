/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>

#include "VppInspect.hpp"

using namespace VPP;

Inspect::Inspect(const std::string &sock_name)
{
    int rc;
    uv_loop_init(&m_server_loop);

    rc = uv_thread_create(&m_server_thread, run, this);
    if (rc < 0) {
        /* throw std::runtime_error(string("Could not create server thread: ") + */
        /*                          uv_strerror(rc)); */
    }
}

void Inspect::run(void* ctx)
{
    Inspect *ins = static_cast<Inspect*>(ctx);
    uv_pipe_t server;

    uv_pipe_init(&ins->m_server_loop, &server, 0);

    // signal(SIGINT, remove_sock);

    int r;
    if ((r = uv_pipe_bind(&server, ins->m_sock_name.c_str())))
    {
        fprintf(stderr, "Bind error %s\n", uv_err_name(r));
        return;
    }
    if ((r = uv_listen((uv_stream_t*) &server, 1, on_connection)))
    {
        fprintf(stderr, "Listen error %s\n", uv_err_name(r));
        return;
    }

    uv_run(&ins->m_server_loop, UV_RUN_DEFAULT);

    assert(false);
}


/* void free_write_req(uv_write_t *req) { */
/*     write_req_t *wr = (write_req_t*) req; */
/*     free(wr->buf.base); */
/*     free(wr); */
/* } */

void alloc_buffer(uv_handle_t *handle,
                  size_t size,
                  uv_buf_t *buf)
{
    buf->base = malloc(size);
    buf->len = size;
}

void echo_write(uv_write_t *req, int status) {
    if (status < 0) {
        fprintf(stderr, "Write error %s\n", uv_err_name(status));
    }
    free_write_req(req);
}

void Inspect::read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread > 0)
    {
        std::string message(buf->base); 
        std::stringstream output;

        Command *cmd = Factory::new_command(message);
        cmd->exec(output);

        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(output.str().c_str(),
                               output.str().length());
        uv_write((uv_write_t*) req, client, &req->buf, 1, echo_write);
        return;
    }

    if (nread < 0)
    {
        if (nread != UV_EOF)
        {
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        }
        uv_close((uv_handle_t*) client, NULL);
    }

    free(buf->base);
}

void Inspect::on_connection(uv_stream_t* server, int status)
{
    if (status == -1)
    {
        // error!
        return;
    }

    uv_pipe_t *client = (uv_pipe_t*) malloc(sizeof(uv_pipe_t));
    uv_pipe_init(loop, client, 0);

    if (uv_accept(server, (uv_stream_t*) client) == 0)
    {
        uv_read_start((uv_stream_t*) client,
                      Inspect::alloc_buffer,
                      Inspect::read);
    }
    else
    {
        uv_close((uv_handle_t*) client, NULL);
    }
}
