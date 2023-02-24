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
#include <bitcoin/network/log/logger.hpp>

#include <utility>
#include <bitcoin/system.hpp>
#include <bitcoin/network/log/timer.hpp>
#include <bitcoin/network/async/async.hpp>
#include <bitcoin/network/boost.hpp>
#include <bitcoin/network/define.hpp>

namespace libbitcoin {
namespace network {

BC_PUSH_WARNING(NO_THROW_IN_NOEXCEPT)

logger::logger() NOEXCEPT
  : pool_(one, thread_priority::low),
    strand_(pool_.service().get_executor()),
    message_subscriber_(strand_),
    event_subscriber_(strand_)
{
}

logger::logger(bool) NOEXCEPT
  : logger()
{
    pool_.stop();
}

logger::~logger() NOEXCEPT
{
    pool_.join();
}

logger::writer logger::write() const NOEXCEPT
{
    return { *this };
}

bool logger::stranded() const NOEXCEPT
{
    return strand_.running_in_this_thread();
}

// messages
// ----------------------------------------------------------------------------

// protected
void logger::notify(const code& ec, std::string&& message) const NOEXCEPT
{
    boost::asio::dispatch(strand_,
        std::bind(&logger::do_notify_message, this, ec, zulu_time(),
            std::move(message)));
}

// private
void logger::do_notify_message(const code& ec, time_t zulu,
    const std::string& message) const NOEXCEPT
{
    BC_ASSERT_MSG(stranded(), "strand");
    message_subscriber_.notify(ec, zulu, message);
}

void logger::subscribe_messages(message_notifier&& handler) NOEXCEPT
{
    boost::asio::dispatch(strand_,
        std::bind(&logger::do_subscribe_messages,
            this, std::move(handler)));
}

// private
void logger::do_subscribe_messages(const message_notifier& handler) NOEXCEPT
{
    BC_ASSERT_MSG(stranded(), "strand");
    message_subscriber_.subscribe(move_copy(handler));
}

// events
// ----------------------------------------------------------------------------

void logger::fire(uint8_t identifier, size_t count) const NOEXCEPT
{
    boost::asio::dispatch(strand_,
        std::bind(&logger::do_notify_event,
            this, identifier, count, fine_clock::now()));
}

// private
void logger::do_notify_event(uint8_t identifier, size_t count,
    const time_point& point) const NOEXCEPT
{
    BC_ASSERT_MSG(stranded(), "strand");
    event_subscriber_.notify(error::success, identifier, count, point);
}

void logger::subscribe_events(event_notifier&& handler) NOEXCEPT
{
    boost::asio::dispatch(strand_,
        std::bind(&logger::do_subscribe_events,
            this, std::move(handler)));
}

// private
void logger::do_subscribe_events(const event_notifier& handler) NOEXCEPT
{
    BC_ASSERT_MSG(stranded(), "strand");
    event_subscriber_.subscribe(move_copy(handler));
}

// stop
// ----------------------------------------------------------------------------

void logger::stop() NOEXCEPT
{
    stop({});
}

void logger::stop(const std::string& message) NOEXCEPT
{
    stop(error::service_stopped, message);
}

void logger::stop(const code& ec, const std::string& message) NOEXCEPT
{
    boost::asio::dispatch(strand_,
        std::bind(&logger::do_stop,
            this, ec, zulu_time(), message));

    pool_.stop();
    BC_DEBUG_ONLY(const auto result =) pool_.join();
    BC_ASSERT_MSG(result, "logger::join");
}

// private
void logger::do_stop(const code& ec, time_t zulu,
    const std::string& message) NOEXCEPT
{
    BC_ASSERT_MSG(stranded(), "strand");

    // Subscriber asserts if stopped with a success code.
    message_subscriber_.stop(ec, zulu, message);
    event_subscriber_.stop(ec, {}, {}, {});
 }

BC_POP_WARNING()

} // namespace network
} // namespace libbitcoin
