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
#include <bitcoin/network/messages/transaction.hpp>

#include <bitcoin/system.hpp>
#include <bitcoin/network/messages/enums/identifier.hpp>
#include <bitcoin/network/messages/enums/level.hpp>
#include <bitcoin/network/messages/message.hpp>

namespace libbitcoin {
namespace network {
namespace messages {

using namespace system;
    
const std::string transaction::command = "tx";
const identifier transaction::id = identifier::transaction;
const uint32_t transaction::version_minimum = level::minimum_protocol;
const uint32_t transaction::version_maximum = level::maximum_protocol;

// static
typename transaction::cptr transaction::deserialize(uint32_t version,
    const system::data_chunk& data, bool witness) NOEXCEPT
{
    read::bytes::copy reader(data);
    const auto message = to_shared(deserialize(version, reader, witness));
    return reader ? message : nullptr;
}

// static
transaction transaction::deserialize(uint32_t version, reader& source,
    bool witness) NOEXCEPT
{
    if (version < version_minimum || version > version_maximum)
        source.invalidate();

    return { to_shared<chain::transaction>(source, witness) };
}

bool transaction::serialize(uint32_t version,
    const system::data_slab& data, bool witness) const NOEXCEPT
{
    write::bytes::copy writer(data);
    serialize(version, writer, witness);
    return writer;
}

void transaction::serialize(uint32_t BC_DEBUG_ONLY(version), writer& sink,
    bool witness) const NOEXCEPT
{
    BC_DEBUG_ONLY(const auto bytes = size(version, witness);)
    BC_DEBUG_ONLY(const auto start = sink.get_write_position();)

    if (transaction_ptr)
        transaction_ptr->to_data(sink, witness);

    BC_ASSERT(sink && sink.get_write_position() - start == bytes);
}

size_t transaction::size(uint32_t, bool witness) const NOEXCEPT
{
    return transaction_ptr ? transaction_ptr->serialized_size(witness) : zero;
}

} // namespace messages
} // namespace network
} // namespace libbitcoin