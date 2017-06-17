/*
 * Test suite for class VppApi
 *
 * Copyright (c) 2017 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_inserter.hpp>

#include <iostream>
#include <queue>

#include "logging.h"
#include "VppOM.hpp"
#include "VppInterface.hpp"
#include "VppL2Interface.hpp"
#include "VppBridgeDomain.hpp"
#include "VppRouteDomain.hpp"
#include "VppVxlanTunnel.hpp"

using namespace boost;
using namespace VPP;

/**
 * An expectation exception
 */
class ExpException
{
public:
    ExpException()
    {
        // a neat place to ad  a brak point
    }
};

class MockCmdQ : public HW::CmdQ
{
public:
    MockCmdQ()
    {
    }
    ~MockCmdQ()
    {
    }
    void expect(Cmd *f)
    {
        m_exp_queue.push(f);
    }
    void enqueue(Cmd *f)
    {
        m_act_queue.push(f);
    }

    bool is_empty()
    {
        return ((0 == m_exp_queue.size()) &&
                (0 == m_act_queue.size()));
    }

    rc_t write()
    {
        Cmd *f_exp, *f_act;
        rc_t rc = rc_t::OK;

        while (m_act_queue.size())
        {
            f_exp = m_exp_queue.front();
            f_act = m_act_queue.front();

            std::cout << " Exp: " << f_exp->to_string() << std::endl;
            std::cout << "  Act: " << f_act->to_string() << std::endl;

            if (typeid(*f_exp) != typeid(*f_act))
            {
                throw ExpException();
            }

            if (typeid(*f_exp) == typeid(Interface::CreateCmd))
            {
                rc = handle_derived<Interface::CreateCmd>(f_exp, f_act);
            }
            else if (typeid(*f_exp) == typeid(Interface::StateChangeCmd))
            {
                rc = handle_derived<Interface::StateChangeCmd>(f_exp, f_act);
            }
            else if (typeid(*f_exp) == typeid(Interface::SetTableCmd))
            {
                rc = handle_derived<Interface::SetTableCmd>(f_exp, f_act);
            }
            else if (typeid(*f_exp) == typeid(Interface::DeleteCmd))
            {
                rc = handle_derived<Interface::DeleteCmd>(f_exp, f_act);
            }
            else if (typeid(*f_exp) == typeid(Interface::PrefixAddCmd))
            {
                rc = handle_derived<Interface::PrefixAddCmd>(f_exp, f_act);
            }
            else if (typeid(*f_exp) == typeid(Interface::PrefixDelCmd))
            {
                rc = handle_derived<Interface::PrefixDelCmd>(f_exp, f_act);
            }
            else if (typeid(*f_exp) == typeid(BridgeDomain::CreateCmd))
            {
                rc = handle_derived<BridgeDomain::CreateCmd>(f_exp, f_act);
            }
            else if (typeid(*f_exp) == typeid(BridgeDomain::DeleteCmd))
            {
                rc = handle_derived<BridgeDomain::DeleteCmd>(f_exp, f_act);
            }
            else if (typeid(*f_exp) == typeid(L2Interface::BindCmd))
            {
                rc = handle_derived<L2Interface::BindCmd>(f_exp, f_act);
            }
            else if (typeid(*f_exp) == typeid(L2Interface::UnbindCmd))
            {
                rc = handle_derived<L2Interface::UnbindCmd>(f_exp, f_act);
            }
            else if (typeid(*f_exp) == typeid(VxlanTunnel::CreateCmd))
            {
                rc = handle_derived<VxlanTunnel::CreateCmd>(f_exp, f_act);
            }
            else if (typeid(*f_exp) == typeid(VxlanTunnel::DeleteCmd))
            {
                rc = handle_derived<VxlanTunnel::DeleteCmd>(f_exp, f_act);
            }
            else
            {
                throw ExpException();
            }

            m_exp_queue.pop();
            m_act_queue.pop();
            delete f_exp;
            delete f_act;

            // return any injected failures to the agent
            if (rc_t::OK != rc)
            {
                return (rc);
            }
        }

        if (m_act_queue.size())
        {
            throw ExpException();
        }

        return (rc);
    }
private:

    template <typename T>
    rc_t handle_derived(const Cmd *f_exp, Cmd *f_act)
    {
        const T *i_exp;
        T *i_act;

        i_exp = dynamic_cast<const T*>(f_exp);
        i_act = dynamic_cast<T*>(f_act);
        if (!(*i_exp == *i_act))
        {
            throw ExpException();
        }
        // pass the data and return code to the agent
        i_act->item() = i_exp->item();

        return (i_act->item().rc());
    }

    // The Q to push the expectations on
    std::queue<Cmd*> m_exp_queue;

    // the queue to push the actual events on
    std::queue<Cmd*> m_act_queue;
};

class VppInit {
public:
    std::string name;
    MockCmdQ *f;

    VppInit()
        : name("vpp-ut"),
          f(new MockCmdQ())
    {
        HW::init(f);
        OM::init();
    }

    ~VppInit() {
        delete f;
    }
};

BOOST_AUTO_TEST_SUITE(VppOM_test)

#define TRY_CHECK_RC(stmt)                    \
{                                             \
    try {                                     \
        BOOST_CHECK(rc_t::OK == stmt);        \
    }                                         \
    catch (ExpException &e)                   \
    {                                         \
        BOOST_CHECK(false);                   \
    }                                         \
    BOOST_CHECK(vi.f->is_empty());            \
}

#define TRY_CHECK(stmt)                       \
{                                             \
    try {                                     \
        stmt;                                 \
    }                                         \
    catch (ExpException &e)                   \
    {                                         \
        BOOST_CHECK(false);                   \
    }                                         \
    BOOST_CHECK(vi.f->is_empty());            \
}

#define ADD_EXPECT(stmt)                      \
    vi.f->expect(new stmt)

BOOST_AUTO_TEST_CASE(interface) {
    VppInit vi;
    const std::string go = "GeorgeOrwell";
    const std::string js = "JohnSteinbeck";
    rc_t rc = rc_t::OK;

    /*
     * George creates and deletes the interface
     */
    std::string itf1_name = "vhost1";
    Interface itf1(itf1_name,
                   Interface::type_t::VHOST,
                   Interface::admin_state_t::UP);

    /*
     * set the expectation for a vhost interface create.
     *  2 is the interface handle VPP [mock] assigns
     */
    HW::Item<handle_t> hw_ifh(2, rc_t::OK);
    ADD_EXPECT(Interface::CreateCmd(hw_ifh, itf1_name, Interface::type_t::VHOST));

    HW::Item<Interface::admin_state_t> hw_as_up(Interface::admin_state_t::UP, rc_t::OK);
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_up, hw_ifh));

    TRY_CHECK_RC(OM::write(go, itf1));

    HW::Item<Interface::admin_state_t> hw_as_down(Interface::admin_state_t::DOWN, rc_t::OK);
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_down, hw_ifh));
    ADD_EXPECT(Interface::DeleteCmd(hw_ifh, Interface::type_t::VHOST));

    TRY_CHECK(OM::remove(go));

    /*
     * George creates the interface, then John brings it down.
     * George's remove is a no-op, sice John also owns the interface
     */
    Interface itf1b(itf1_name,
                    Interface::type_t::VHOST,
                    Interface::admin_state_t::DOWN);

    ADD_EXPECT(Interface::CreateCmd(hw_ifh, itf1_name, Interface::type_t::VHOST));
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_up, hw_ifh));
    TRY_CHECK_RC(OM::write(go, itf1));

    ADD_EXPECT(Interface::StateChangeCmd(hw_as_down, hw_ifh));
    TRY_CHECK_RC(OM::write(js, itf1b));

    TRY_CHECK(OM::remove(go));

    ADD_EXPECT(Interface::DeleteCmd(hw_ifh, Interface::type_t::VHOST));
    TRY_CHECK(OM::remove(js));

    /*
     * George adds an interface, then we flush all of Geroge's state
     */
    ADD_EXPECT(Interface::CreateCmd(hw_ifh, itf1_name, Interface::type_t::VHOST));
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_up, hw_ifh));
    TRY_CHECK_RC(OM::write(go, itf1));

    TRY_CHECK(OM::mark(go));

    ADD_EXPECT(Interface::StateChangeCmd(hw_as_down, hw_ifh));
    ADD_EXPECT(Interface::DeleteCmd(hw_ifh, Interface::type_t::VHOST));
    TRY_CHECK(OM::sweep(go));

    /*
     * George adds an interface. mark stale. update the same interface. sweep
     * and expect no delete
     */
    ADD_EXPECT(Interface::CreateCmd(hw_ifh, itf1_name, Interface::type_t::VHOST));
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_down, hw_ifh));
    TRY_CHECK_RC(OM::write(go, itf1b));

    TRY_CHECK(OM::mark(go));

    ADD_EXPECT(Interface::StateChangeCmd(hw_as_up, hw_ifh));
    TRY_CHECK_RC(OM::write(go, itf1));

    TRY_CHECK(OM::sweep(go));

    ADD_EXPECT(Interface::StateChangeCmd(hw_as_down, hw_ifh));
    ADD_EXPECT(Interface::DeleteCmd(hw_ifh, Interface::type_t::VHOST));
    TRY_CHECK(OM::remove(go));

    /*
     * George adds an insterface, then we mark that state. Add a second interface
     * an flush the first that is now stale.
     */
    ADD_EXPECT(Interface::CreateCmd(hw_ifh, itf1_name, Interface::type_t::VHOST));
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_up, hw_ifh));
    TRY_CHECK_RC(OM::write(go, itf1));

    TRY_CHECK(OM::mark(go));

    std::string itf2_name = "vhost2";
    Interface itf2(itf2_name,
                   Interface::type_t::VHOST,
                   Interface::admin_state_t::UP);
    HW::Item<handle_t> hw_ifh2(3, rc_t::OK);

    ADD_EXPECT(Interface::CreateCmd(hw_ifh2, itf2_name, Interface::type_t::VHOST));
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_up, hw_ifh2));
    TRY_CHECK_RC(OM::write(go, itf2));

    ADD_EXPECT(Interface::StateChangeCmd(hw_as_down, hw_ifh));
    ADD_EXPECT(Interface::DeleteCmd(hw_ifh, Interface::type_t::VHOST));
    TRY_CHECK(OM::sweep(go));

    TRY_CHECK(OM::mark(go));

    ADD_EXPECT(Interface::StateChangeCmd(hw_as_down, hw_ifh2));
    ADD_EXPECT(Interface::DeleteCmd(hw_ifh2, Interface::type_t::VHOST));
    TRY_CHECK(OM::sweep(go));

    /*
     * A BVI interface with an IP prefix configured.
     */
}

BOOST_AUTO_TEST_CASE(bvi) {
    VppInit vi;
    const std::string ernest = "ErnestHemmingway";
    const std::string graham = "GrahamGreene";
    rc_t rc = rc_t::OK;

    HW::Item<Interface::admin_state_t> hw_as_up(Interface::admin_state_t::UP,
                                                rc_t::OK);
    HW::Item<Interface::admin_state_t> hw_as_down(Interface::admin_state_t::DOWN,
                                                  rc_t::OK);

    /*
     * Enrest creates a BVI with address 10.10.10.10/24
     */
    Route::prefix_t pfx_10("10.10.10.10", 24);

    const std::string bvi_name = "bvi1";
    Interface itf(bvi_name,
                  Interface::type_t::BVI,
                  Interface::admin_state_t::UP,
                  pfx_10);
    HW::Item<handle_t> hw_ifh(4, rc_t::OK);
    HW::Item<Route::prefix_t> hw_pfx_10(pfx_10, rc_t::OK);

    ADD_EXPECT(Interface::CreateCmd(hw_ifh, bvi_name, Interface::type_t::BVI));
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_up, hw_ifh));
    ADD_EXPECT(Interface::PrefixAddCmd(hw_pfx_10, hw_ifh));

    TRY_CHECK_RC(OM::write(ernest, itf));

    ADD_EXPECT(Interface::PrefixDelCmd(hw_pfx_10, hw_ifh));
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_down, hw_ifh));
    ADD_EXPECT(Interface::DeleteCmd(hw_ifh, Interface::type_t::BVI));
    TRY_CHECK(OM::remove(ernest));

    /*
     * Graham creates a BVI with address 10.10.10.10/24 in Routing Domain
     */

    RouteDomain rd("red");
    HW::Item<Route::table_id_t> hw_rd_bind(1, rc_t::OK);
    HW::Item<Route::table_id_t> hw_rd_unbind(0, rc_t::OK);
    TRY_CHECK_RC(OM::write(graham, rd));

    const std::string bvi2_name = "bvi2";
    Interface itf2(bvi2_name,
                   Interface::type_t::BVI,
                   Interface::admin_state_t::UP,
                   rd,
                   pfx_10);
    HW::Item<handle_t> hw_ifh2(5, rc_t::OK);

    ADD_EXPECT(Interface::CreateCmd(hw_ifh2, bvi2_name, Interface::type_t::BVI));
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_up, hw_ifh2));
    ADD_EXPECT(Interface::SetTableCmd(hw_rd_bind, hw_ifh2));
    ADD_EXPECT(Interface::PrefixAddCmd(hw_pfx_10, hw_ifh2));

    TRY_CHECK_RC(OM::write(graham, itf2));

    ADD_EXPECT(Interface::PrefixDelCmd(hw_pfx_10, hw_ifh2));
    ADD_EXPECT(Interface::SetTableCmd(hw_rd_unbind, hw_ifh2));
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_down, hw_ifh2));
    ADD_EXPECT(Interface::DeleteCmd(hw_ifh2, Interface::type_t::BVI));
    TRY_CHECK(OM::remove(graham));
}

BOOST_AUTO_TEST_CASE(bridge) {
    VppInit vi;
    const std::string franz = "FranzKafka";
    const std::string dante = "Dante";
    rc_t rc = rc_t::OK;

    /*
     * Franz creates an interface, Bridge-domain, then binds the two
     */

    // interface create
    std::string itf1_name = "vhost1";
    Interface itf1(itf1_name,
                   Interface::type_t::VHOST,
                   Interface::admin_state_t::UP);

    HW::Item<handle_t> hw_ifh(3, rc_t::OK);
    HW::Item<Interface::admin_state_t> hw_as_up(Interface::admin_state_t::UP,
                                                rc_t::OK);
    ADD_EXPECT(Interface::CreateCmd(hw_ifh, itf1_name, Interface::type_t::VHOST));
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_up, hw_ifh));

    TRY_CHECK_RC(OM::write(franz, itf1));

    // bridge-domain create
    std::string bd1_name = "bd1";
    BridgeDomain bd1(bd1_name);

    HW::Item<handle_t> hw_bd(33, rc_t::OK);
    ADD_EXPECT(BridgeDomain::CreateCmd(hw_bd, bd1_name));

    TRY_CHECK_RC(OM::write(franz, bd1));

    // L2-interface create and bind
    // this needs to be delete'd before the flush below, since it too maintains
    // references to the BD and Interface
    L2Interface *l2itf = new L2Interface(itf1, bd1);
    HW::Item<bool> hw_l2_bind(true, rc_t::OK);

    ADD_EXPECT(L2Interface::BindCmd(hw_l2_bind, hw_ifh.data(), hw_bd.data(), false));
    TRY_CHECK_RC(OM::write(franz, *l2itf));

    /*
     * Dante adds an interface to the same BD
     */
    std::string itf2_name = "vhost2";
    Interface itf2(itf2_name,
                   Interface::type_t::VHOST,
                   Interface::admin_state_t::UP);

    HW::Item<handle_t> hw_ifh2(4, rc_t::OK);
    ADD_EXPECT(Interface::CreateCmd(hw_ifh2, itf2_name, Interface::type_t::VHOST));
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_up, hw_ifh2));
    TRY_CHECK_RC(OM::write(dante, itf2));

    // BD add is a no-op since it exists
    TRY_CHECK_RC(OM::write(dante, bd1));

    L2Interface *l2itf2 = new L2Interface(itf2, bd1);

    ADD_EXPECT(L2Interface::BindCmd(hw_l2_bind, hw_ifh2.data(), hw_bd.data(), false));
    TRY_CHECK_RC(OM::write(dante, *l2itf2));

    // flush Franz's state
    delete l2itf;
    HW::Item<Interface::admin_state_t> hw_as_down(Interface::admin_state_t::DOWN,
                                                  rc_t::OK);
    ADD_EXPECT(L2Interface::UnbindCmd(hw_l2_bind, hw_ifh.data(), hw_bd.data(), false));
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_down, hw_ifh));
    ADD_EXPECT(Interface::DeleteCmd(hw_ifh, Interface::type_t::VHOST));
    TRY_CHECK(OM::remove(franz));

    // flush Dante's state - the fact the BD is removed after the interface
    // is an uncontrollable artifact of the C++ object destruction.
    delete l2itf2;
    ADD_EXPECT(L2Interface::UnbindCmd(hw_l2_bind, hw_ifh2.data(), hw_bd.data(), false));
    ADD_EXPECT(Interface::StateChangeCmd(hw_as_down, hw_ifh2));
    ADD_EXPECT(Interface::DeleteCmd(hw_ifh2, Interface::type_t::VHOST));
    ADD_EXPECT(BridgeDomain::DeleteCmd(hw_bd));
    TRY_CHECK(OM::remove(dante));
}

BOOST_AUTO_TEST_CASE(vxlan) {
    VppInit vi;
    const std::string franz = "FranzKafka";
    rc_t rc = rc_t::OK;

    /*
     * Franz creates an interface, Bridge-domain, then binds the two
     */

    // VXLAN create
    boost::asio::ip::address src = boost::asio::ip::address::from_string("10.10.10.10");
    boost::asio::ip::address dst = boost::asio::ip::address::from_string("10.10.10.11");
    uint32_t vni;

    VxlanTunnel vxt(src, dst, vni);

    HW::Item<handle_t> hw_vxt(3, rc_t::OK);
    ADD_EXPECT(VxlanTunnel::CreateCmd(hw_vxt, src, dst, vni));

    TRY_CHECK_RC(OM::write(franz, vxt));

    // bridge-domain create
    std::string bd1_name = "bd1";
    BridgeDomain bd1(bd1_name);

    HW::Item<handle_t> hw_bd(33, rc_t::OK);
    ADD_EXPECT(BridgeDomain::CreateCmd(hw_bd, bd1_name));

    TRY_CHECK_RC(OM::write(franz, bd1));

    // L2-interface create and bind
    // this needs to be delete'd before the flush below, since it too maintains
    // references to the BD and Interface
    L2Interface *l2itf = new L2Interface(vxt, bd1);
    HW::Item<bool> hw_l2_bind(true, rc_t::OK);

    ADD_EXPECT(L2Interface::BindCmd(hw_l2_bind, hw_vxt.data(), hw_bd.data(), false));
    TRY_CHECK_RC(OM::write(franz, *l2itf));

    // flush Franz's state
    delete l2itf;
    HW::Item<handle_t> hw_vxtdel(3, rc_t::NOOP);
    ADD_EXPECT(L2Interface::UnbindCmd(hw_l2_bind, hw_vxt.data(), hw_bd.data(), false));
    ADD_EXPECT(BridgeDomain::DeleteCmd(hw_bd));
    ADD_EXPECT(VxlanTunnel::DeleteCmd(hw_vxtdel, src, dst, vni));
    TRY_CHECK(OM::remove(franz));
}

BOOST_AUTO_TEST_SUITE_END()
