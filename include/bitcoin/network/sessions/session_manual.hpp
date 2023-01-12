/**
 * Copyright (c) 2011-2019 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_NETWORK_SESSION_MANUAL_HPP
#define LIBBITCOIN_NETWORK_SESSION_MANUAL_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <bitcoin/system.hpp>
#include <bitcoin/network/config/config.hpp>
#include <bitcoin/network/define.hpp>
#include <bitcoin/network/net/net.hpp>
#include <bitcoin/network/sessions/session.hpp>
#include <bitcoin/network/settings.hpp>

namespace libbitcoin {
namespace network {

class p2p;

/// Manual connections session, thread safe.
class BCT_API session_manual
  : public session, track<session_manual>
{
public:
    typedef std::shared_ptr<session_manual> ptr;
    typedef std::function<void(const code&, const channel::ptr&)>
        channel_handler;

    /// Construct an instance (network should be started).
    session_manual(p2p& network) NOEXCEPT;

    /// Start the session of persistent connections (call from network strand).
    void start(result_handler&& handler) NOEXCEPT override;

    /// Connect.
    /// ------------------------------------------------------------------------
    /// Establish a persistent connection, call from network strand.

    /////// Maintain connection with callback on each connection attempt and stop.
    ////virtual void connect(const config::authority& peer,
    ////    channel_handler&& handler) NOEXCEPT;

    /// Maintain connection to a node until session stop.
    virtual void connect(const config::endpoint& endpoint) NOEXCEPT;

    /// Maintain connection with callback on each connection attempt and stop.
    virtual void connect(const config::endpoint& endpoint,
        channel_handler&& handler) NOEXCEPT;

protected:
    /// The channel is outbound (do not pend the nonce).
    bool inbound() const NOEXCEPT override;

    /// Notify subscribers on channel start.
    bool notify() const NOEXCEPT override;

    /// Overridden to change version protocol (base calls from channel strand).
    void attach_handshake(const channel::ptr& channel,
        result_handler&& handler) const NOEXCEPT override;

    /// Overridden to change channel protocols (base calls from channel strand).
    void attach_protocols(const channel::ptr& channel) const NOEXCEPT override;

    /// Start or restart the given connection (called from connect).
    virtual void start_connect(const config::endpoint& peer,
        const connector::ptr& connector, const channel_handler& handler) NOEXCEPT;

private:
    void handle_started(const code& ec, const result_handler& handler) NOEXCEPT;
    void handle_connect(const code& ec, const channel::ptr& channel,
        const config::endpoint& peer, const connector::ptr& connector,
        const channel_handler& handler) NOEXCEPT;

    void handle_channel_start(const code& ec, const config::endpoint& peer,
        const channel::ptr& channel, const channel_handler& handler) NOEXCEPT;
    void handle_channel_stop(const code& ec, const config::endpoint& peer,
        const connector::ptr& connector, const channel_handler& handler) NOEXCEPT;
};

} // namespace network
} // namespace libbitcoin

#endif
