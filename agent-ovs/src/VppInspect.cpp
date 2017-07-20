/*
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <cassert>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>
//#include <boost/algorithm/string/trim.hpp>

#include "VppInspect.hpp"
#include "VppInterface.hpp"
#include "VppBridgeDomain.hpp"
#include "VppRouteDomain.hpp"
#include "VppL3Config.hpp"
#include "VppL2Config.hpp"
#include "VppDhcpConfig.hpp"
#include "VppAclBinding.hpp"

using namespace VPP;

std::unique_ptr<std::map<std::string, Inspect::CommandHandler*>> Inspect::m_cmd_handlers;

std::unique_ptr<std::deque<std::pair<std::vector<std::string>, std::string>>> Inspect::m_help_handlers;

Inspect::Inspect(const std::string &sock_name):
    m_sock_name(sock_name)
{
    int rc;
    uv_loop_init(&m_server_loop);
    m_server_loop.data = this;

    rc = uv_async_init(&m_server_loop, &m_async, Inspect::on_cleanup);

    rc = uv_thread_create(&m_server_thread, run, this);
    if (rc < 0)
    {
        LOG(ovsagent::ERROR) << "Inspect - thread create error:" << uv_strerror(rc);
    }
}

Inspect::~Inspect()
{
    uv_async_send(&m_async);
    uv_thread_join(&m_server_thread);
    uv_loop_close(&m_server_loop);

    LOG(ovsagent::INFO) << "Inspect - close";
}

void Inspect::on_cleanup(uv_async_t* handle)
{
    Inspect *ins = static_cast<Inspect*>(handle->loop->data);

    uv_stop(&ins->m_server_loop);
}

void Inspect::run(void* ctx)
{
    Inspect *ins = static_cast<Inspect*>(ctx);
    uv_pipe_t server;
    int rv;

    uv_pipe_init(&ins->m_server_loop, &server, 0);

    LOG(ovsagent::INFO) << "Inspect - open:" << ins->m_sock_name;

    if ((rv = uv_pipe_bind(&server, ins->m_sock_name.c_str())))
    {
        LOG(ovsagent::ERROR) << "Inspect - Bind error:" << uv_err_name(rv);
        return;
    }
    if ((rv = uv_listen((uv_stream_t*) &server, 1, on_connection)))
    {
        LOG(ovsagent::ERROR) << "Inspect - Listen error:" << uv_err_name(rv);
        return;
    }

    uv_run(&ins->m_server_loop, UV_RUN_DEFAULT);
    uv_close((uv_handle_t*)&server, NULL);
}


Inspect::write_req_t::~write_req_t()
{
    free(buf.base);
}

Inspect::write_req_t::write_req_t(std::ostringstream &output)
{
    buf.len = output.str().length();
    buf.base = (char*) malloc(buf.len);
    memcpy(buf.base,
           output.str().c_str(),
           buf.len);
}

void Inspect::on_alloc_buffer(uv_handle_t *handle,
                              size_t size,
                              uv_buf_t *buf)
{
    buf->base = (char*) malloc(size);
    buf->len = size;
}

void Inspect::on_write(uv_write_t *req,
                       int status)
{
    write_req_t *wr = (write_req_t*) req;

    if (status < 0)
    {
        LOG(ovsagent::ERROR) << "Inspect - Write error:" << uv_err_name(status);
    }

    delete req;
}

void Inspect::do_write(uv_stream_t *client,
                       std::ostringstream &output)
{
    write_req_t *req = new write_req_t(output);

    uv_write((uv_write_t*) req, client, &req->buf, 1, on_write);
}

void Inspect::on_read(uv_stream_t *client,
                      ssize_t nread,
                      const uv_buf_t *buf)
{
    if (nread > 0)
    {
        std::ostringstream output;
        std::string message(buf->base);
        message = message.substr(0, nread);
        boost::trim(message);

        if (message.length())
        {
            if (message.find("help") != std::string::npos)
            {
                output << "Command Options: " << std::endl;
                output << "  keys              - Show all keys owning objects"  << std::endl;
                output << "  key:XXX           - Show all object referenced by key XXX"  << std::endl;
                output << "  all               - Show All objects" << std::endl;
                output << "Individual Object Types:" << std::endl;

                for (auto h : *m_help_handlers)
                {
                    output << "  {";

                    for (auto s: h.first)
                    {
                        output << s << " ";
                    }
                    output << "} - \t";
                    output << h.second;
                    output << std::endl;
                }
            }
            else if (message.find("keys") != std::string::npos)
            {
                OM::dump(output);
            }
            else if (message.find("key") != std::string::npos)
            {
                std::vector<std::string> results;
                boost::split(results, message, boost::is_any_of(":\n"));

                OM::dump(results[1], output);
            }
            else if (message.find("all") != std::string::npos)
            {
                /*
                 * get the unique set of handlers, then invoke each
                 */
                std::set<CommandHandler*> hdlrs;
                for (auto h : *m_cmd_handlers)
                {
                    hdlrs.insert(h.second);
                }
                for (auto h : hdlrs)
                {
                    h->show(output);
                }
            }
            else
            {
                auto it = m_cmd_handlers->find(message);

                if (it != m_cmd_handlers->end())
                {
                    it->second->show(output);
                }
                else
                {
                    output << "Unknown Command: " << message << std::endl;
                }
            }
        }
        output << "# ";

        do_write((uv_stream_t*) client, output);
    }
    else if (nread < 0)
    {
        if (nread != UV_EOF)
        {
            LOG(ovsagent::ERROR) << "Inspect - Read error:" << uv_err_name(nread);
        }
        uv_close((uv_handle_t*) client, NULL);
    }

    free(buf->base);
}

void Inspect::on_connection(uv_stream_t* server,
                            int status)
{
    Inspect *ins = static_cast<Inspect*>(server->loop->data);

    if (status == -1)
    {
        // error!
        return;
    }

    uv_pipe_t *client = (uv_pipe_t*) malloc(sizeof(uv_pipe_t));
    uv_pipe_init(&ins->m_server_loop, client, 0);

    if (uv_accept(server, (uv_stream_t*) client) == 0)
    {
        std::ostringstream output;

        output << "Welcome: VPP Inspect" << std::endl;
        output << "# ";

        do_write((uv_stream_t*) client, output);

        uv_read_start((uv_stream_t*) client,
                      Inspect::on_alloc_buffer,
                      Inspect::on_read);
    }
    else
    {
        uv_close((uv_handle_t*) client, NULL);
    }
}

void Inspect::register_handler(const std::vector<std::string> &cmds,
                               const std::string &help,
                               CommandHandler *handler)
{
    if (!m_cmd_handlers)
    {
        m_cmd_handlers.reset(new std::map<std::string, CommandHandler*>);
        m_help_handlers.reset(new std::deque<std::pair<std::vector<std::string>,
                                                       std::string>>);
    }

    for (auto cmd : cmds)
    {
        (*m_cmd_handlers)[cmd] = handler;
    }
    m_help_handlers->push_front(std::make_pair(cmds, help));
}
