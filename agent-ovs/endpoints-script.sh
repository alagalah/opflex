#!/usr/bin/env bash

# create four endpoints in four different namespaces
# bind four endpoints with the VPP
# try to ping from one another

sudo ip netns add ns1
sudo ip netns add ns2
sudo ip netns add ns3
sudo ip netns add ns4
#sudo ip netns add ns5

sudo ip link add veth1 type veth peer name veth2
sudo ip link add veth3 type veth peer name veth4
sudo ip link add veth5 type veth peer name veth6
sudo ip link add veth7 type veth peer name veth8
#sudo ip link add veth9 type veth peer name veth10
#sudo ip link add veth11 type veth peer name veth12

sudo ip link set veth1 up
sudo ip link set veth3 up
sudo ip link set veth5 up
sudo ip link set veth7 up
#sudo ip link set veth9 up
#sudo ip link set veth10 up
#sudo ip link set veth11 up

sudo ip link set veth2 netns ns1
sudo ip link set veth4 netns ns2
sudo ip link set veth6 netns ns3
sudo ip link set veth8 netns ns4
#sudo ip link set veth12 netns ns5

sudo ip netns exec ns1 ip link set veth2 up
sudo ip netns exec ns2 ip link set veth4 up
sudo ip netns exec ns3 ip link set veth6 up
sudo ip netns exec ns4 ip link set veth8 up
#sudo ip netns exec ns5 ip link set veth12 up


sudo ip netns exec ns1 ip addr add 10.0.0.1/24 dev veth2
sudo ip netns exec ns2 ip addr add 10.0.0.2/24 dev veth4
sudo ip netns exec ns3 ip addr add 10.0.1.1/24 dev veth6
sudo ip netns exec ns4 ip addr add 10.0.1.2/24 dev veth8
#sudo ip netns exec ns1 ip addr add 10.0.0.3/24 dev veth10


#These are for BVI interfaces
#sudo ip netns exec ns3 ip route add 10.0.0.0/24 via 10.0.1.128
#sudo ip netns exec ns4 ip route add 10.0.0.0/24 via 10.0.1.128
#sudo ip netns exec ns1 ip route add 10.0.1.0/24 via 10.0.0.128
#sudo ip netns exec ns2 ip route add 10.0.1.0/24 via 10.0.0.128
#sudo ip netns exec ns5 ip route add 10.0.0.0/24 via 10.0.1.128

echo BLDLOG: create endpoints
cat <<__EE__ > /usr/local/var/lib/opflex-agent-ovs/endpoints/h1.ep
licy-space-name": "test",
    "endpoint-group-name": "group1",
    "interface-name": "veth1",
    "ip": [
        "10.0.0.1"
    ],
    "mac": "00:00:00:00:00:01",
    "uuid": "83f18f0b-80f7-46e2-b06c-4d9487b0c754"
}
__EE__

cat <<__EE__ > /usr/local/var/lib/opflex-agent-ovs/endpoints/h2.ep
{
    "policy-space-name": "test",
    "endpoint-group-name": "group1",
    "interface-name": "veth3",
    "ip": [
        "10.0.0.2"
    ],
    "mac": "00:00:00:00:00:02",
    "uuid": "83f18f0b-80f7-46e2-b06c-4d9487b0c755"
}
__EE__

cat <<__EE__ > /usr/local/var/lib/opflex-agent-ovs/endpoints/h3.ep
{
    "policy-space-name": "test",
    "endpoint-group-name": "group3",
    "interface-name": "veth5",
    "ip": [
        "10.0.1.1"
    ],
    "mac": "00:00:00:00:00:03",
    "uuid": "83f18f0b-80f7-46e2-b06c-4d9487b0c756"
}

__EE__

cat <<__EE__ > /usr/local/var/lib/opflex-agent-ovs/endpoints/h4.ep
{
    "policy-space-name": "test",
    "endpoint-group-name": "group3",
    "interface-name": "veth7",
    "ip": [
        "10.0.1.2"
    ],
    "mac": "00:00:00:00:00:04",
    "uuid": "83f18f0b-80f7-46e2-b06c-4d9487b0c757"
}

__EE__
