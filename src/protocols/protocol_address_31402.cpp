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
#include <bitcoin/network/protocols/protocol_address_31402.hpp>

#include <functional>
#include <string>
#include <bitcoin/system.hpp>
#include <bitcoin/network/define.hpp>
#include <bitcoin/network/messages/messages.hpp>
#include <bitcoin/network/net/net.hpp>
#include <bitcoin/network/p2p.hpp>
#include <bitcoin/network/protocols/protocol.hpp>
#include <bitcoin/network/protocols/protocol_events.hpp>

namespace libbitcoin {
namespace network {

#define CLASS protocol_address_31402
static const std::string protocol_name = "address";

using namespace bc::system;
using namespace messages;
using namespace std::placeholders;

static messages::address configured_self(const network::settings& settings)
{
    if (settings.self.port() == 0)
        return {};

    return { { settings.self.to_address_item() } };
}

protocol_address_31402::protocol_address_31402(channel::ptr channel,
    p2p& network)
  : protocol_events(channel),
    network_(network),
    self_(configured_self(network_.network_settings()))
{
}

// Start sequence.
// ----------------------------------------------------------------------------

void protocol_address_31402::start()
{
    const auto& settings = network_.network_settings();

    // Events start completes without invoking the handler.
    // Must have a handler to capture a shared self pointer in stop subscriber.
    protocol_events::start(BIND1(handle_stop, _1));

    if (!self_.addresses.empty())
    {
        SEND2(self_, handle_send, _1, self_.command);
    }

    // If we can't store addresses we don't ask for or handle them.
    if (settings.host_pool_capacity == 0)
        return;

    SUBSCRIBE2(address, handle_receive_address, _1, _2);
    SUBSCRIBE2(get_address, handle_receive_get_address, _1, _2);
    SEND2(get_address{}, handle_send, _1, get_address::command);
}

// Protocol.
// ----------------------------------------------------------------------------

bool protocol_address_31402::handle_receive_address(const code& ec,
    address::ptr message)
{
    if (stopped(ec))
        return false;

    LOG_VERBOSE(LOG_NETWORK)
        << "Storing addresses from [" << authority() << "] ("
        << message->addresses.size() << ")";

    // TODO: manage timestamps (active channels are connected < 3 hours ago).
////    network_.store(message->addresses, BIND1(handle_store_addresses, _1));

    // RESUBSCRIBE
    return true;
}

bool protocol_address_31402::handle_receive_get_address(const code& ec,
    get_address::ptr)
{
    if (stopped(ec))
        return false;

    messages::address_items subset;
////    const auto code = network_.fetch_addresses(subset);
code code;

    if (!code)
    {
        SEND2(address{ subset }, handle_send, _1, self_.command);

        LOG_DEBUG(LOG_NETWORK)
            << "Sending addresses to [" << authority() << "] ("
            << self_.addresses.size() << ")";
    }

    // do not resubscribe; one response per connection permitted
    return false;
}

void protocol_address_31402::handle_store_addresses(const code& ec)
{
    if (stopped(ec))
        return;

    if (ec)
    {
        LOG_ERROR(LOG_NETWORK)
            << "Failure storing addresses from [" << authority() << "] "
            << ec.message();
        stop(ec);
    }
}

void protocol_address_31402::handle_stop(const code&)
{
    // None of the other bc::network protocols log their stop.
    ////LOG_DEBUG(LOG_NETWORK)
    ////    << "Stopped address protocol for [" << authority() << "].";
}

const std::string& protocol_address_31402::name() const
{
    return protocol_name;
}

} // namespace network
} // namespace libbitcoin
