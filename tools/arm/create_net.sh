#!/bin/bash

if test $# -ne 1; then
    echo "Usage: $0 <device>" 1>&2
    echo " <device> must be tap0 or tap1"
    exit 1
fi

device=$1

case "$device" in
    "tap0")
        subnet=172.213.0
        ;;
    "tap1")
        subnet=172.214.0
        ;;
    *)
        echo "Unknown device" 1>&2
        exit 1
        ;;
esac

sudo ip link del dev $device

# If device doesn't exist add device.
if ! /sbin/ip link show dev "$device" > /dev/null 2>&1; then
    sudo ip tuntap add mode tap user "$USER" dev "$device"
fi

# Reconfigure just to be sure (even if device exists).
sudo /sbin/ip address flush dev "$device"
sudo /sbin/ip link set dev "$device" down
sudo /sbin/ip address add $subnet.1/24 dev "$device"
sudo /sbin/ip link set dev "$device" up

mkdir -p $PWD/tftp

sudo killall -9 dnsmasq
sudo dnsmasq --enable-tftp --tftp-root=$PWD/tftp --no-resolv --no-hosts --bind-interfaces --interface $device -F $subnet.2,$subnet.20 -x dnsmasq.pid || true

sudo sysctl net.ipv4.ip_forward=1
sudo sysctl net.ipv6.conf.default.forwarding=1
sudo sysctl net.ipv6.conf.all.forwarding=1

interface_name=wlp2s0

sudo iptables -t nat -A POSTROUTING -o $interface_name -j MASQUERADE
sudo iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
sudo iptables -A FORWARD -i $device -o $interface_name -j ACCEPT
