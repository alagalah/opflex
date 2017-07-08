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

#include "VppInspect.hpp"
#include "VppInterface.hpp"
#include "VppBridgeDomain.hpp"
#include "VppRouteDomain.hpp"
#include "VppL3Config.hpp"
#include "VppL2Config.hpp"

using namespace VPP;

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
    buf = uv_buf_init(output.str().c_str(),
                      output.str().length());
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
        std::string message(buf->base); 
        std::ostringstream output;

        Command *cmd = new_command(message);

        if (cmd)
        {
            cmd->exec(output);
            do_write(client, output);

            delete cmd;
        }
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
        ShowHelp sh;

        output << "Welcome: VPP Inspect" << std::endl;
        sh.exec(output);
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

Inspect::Command * Inspect::new_command(const std::string &message)
{
    if (message.find("inst") != std::string::npos)
    {
        if ((message.find("interface") != std::string::npos) ||
            (message.find("intf") != std::string::npos))
        {
            return new ShowInterface();
        }
        else if (message.find("bridge") != std::string::npos)
        {
            return new ShowBridgeDomain();
        }
        else if (message.find("route") != std::string::npos)
        {
            return new ShowBridgeDomain();
        }
        else if ((message.find("L2Config") != std::string::npos) ||
                 (message.find("l2") != std::string::npos))
        {
            return new ShowL2Config();
        } 
        else if ((message.find("L23onfig") != std::string::npos) ||
                 (message.find("l3") != std::string::npos))
        {
            return new ShowL3Config();
        }
        else if (message.find("all"))
        {
            return new ShowAll();
        }
    }
    else if (message.find("key") != std::string::npos)
    {
        std::vector<std::string> results;
        boost::split(results, message, boost::is_any_of(":\n"));

        return new ShowKey(results[1]);
    }
    else if (message.find("help") != std::string::npos)
    {
        return new ShowHelp();
    }
    return (nullptr);
}

void Inspect::ShowHelp::exec(std::ostream &os)
{
    os << "Command Options: "                          << std::endl;
    os << " inst:all        - Show all objects"        << std::endl;
    os << " inst:interface  - Show all interfaces"     << std::endl;
    os << " inst:bridge     - Show all Bridge-Domain"  << std::endl;
    os << " inst:route      - Show all Route-Domaina"  << std::endl;
    os << " inst:L3Config   - Show all L3 Configs"     << std::endl;
    os << " inst:L2Config   - Show all L2 Configs"     << std::endl;
    os << " key:XXX         - Show all object referenced by key XXX"  << std::endl;
    os << "                    where XXX is gleaned from gbp_inspect" << std::endl;
    os << std::endl;
}

void Inspect::ShowInterface::exec(std::ostream &os)
{
    Interface::dump(os);
}

void Inspect::ShowBridgeDomain::exec(std::ostream &os)
{
    BridgeDomain::dump(os);
}

void Inspect::ShowL2Config::exec(std::ostream &os)
{
    L2Config::dump(os);
}

void Inspect::ShowL3Config::exec(std::ostream &os)
{
    L3Config::dump(os);
}

void Inspect::ShowRouteDomain::exec(std::ostream &os)
{
    RouteDomain::dump(os);
}

void Inspect::ShowAll::exec(std::ostream &os)
{
    Interface::dump(os);
    BridgeDomain::dump(os);
    L2Config::dump(os);
    RouteDomain::dump(os);
    L3Config::dump(os);
}

void Inspect::ShowKey::exec(std::ostream &os)
{
    OM::dump(m_key, os);
}

Inspect::ShowKey::ShowKey(const std::string &key):
    m_key(key)
{
}
