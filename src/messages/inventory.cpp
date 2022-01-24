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
#include <bitcoin/network/messages/inventory.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <bitcoin/system.hpp>
#include <bitcoin/network/messages/enums/identifier.hpp>
#include <bitcoin/network/messages/enums/level.hpp>
#include <bitcoin/network/messages/inventory_item.hpp>
#include <bitcoin/network/messages/message.hpp>

namespace libbitcoin {
namespace network {
namespace messages {

using namespace bc::system;
    
// Multiple inv message in reply enabled by bip61.
const std::string inventory::command = "inv";
const identifier inventory::id = identifier::inventory;
const uint32_t inventory::version_minimum = level::minimum_protocol;
const uint32_t inventory::version_maximum = level::maximum_protocol;

// static
inventory inventory::factory(hash_list&& hashes, type_id type)
{
    static default_allocator<inventory_item> no_fill_allocator{};

    inventory_items items(no_fill_allocator);
    items.resize(hashes.size());

    std::transform(hashes.begin(), hashes.end(), items.begin(),
        [=](hash_digest& hash)
        {
            return inventory_item{ type, std::move(hash) };
        });

    return { items };
}

inventory inventory::factory(const hash_list& hashes, type_id type)
{
    static default_allocator<inventory_item> no_fill_allocator{};

    inventory_items items(no_fill_allocator);
    items.resize(hashes.size());

    std::transform(hashes.begin(), hashes.end(), items.begin(),
        [=](const hash_digest& hash)
        {
            return inventory_item{ type, hash };
        });

    return { items };
}

inventory inventory::deserialize(uint32_t version, reader& source)
{
    if (version < version_minimum || version > version_maximum)
        source.invalidate();

    inventory_items items;
    items.resize(source.read_size(max_inventory));

    for (size_t item = 0; item < items.capacity(); ++item)
        items.push_back(inventory_item::deserialize(version, source));

    return { items };
}

void inventory::serialize(uint32_t version, writer& sink) const
{
    BC_DEBUG_ONLY(const auto bytes = size(version);)
    BC_DEBUG_ONLY(const auto start = sink.get_position();)

    sink.write_variable(items.size());

    for (const auto& item: items)
        item.serialize(version, sink);

    BC_ASSERT(sink && sink.get_position() - start == bytes);
}

size_t inventory::size(uint32_t version) const
{
    return variable_size(items.size()) +
        (items.size() * inventory_item::size(version));
}

inventory_items inventory::filter(type_id type) const
{
    inventory_items out;
    out.reserve(count(type));

    for (const auto& item: items)
        if (item.type == type)
            out.push_back(item);

    return out;
}

hash_list inventory::to_hashes(type_id type) const
{
    hash_list out;
    out.reserve(count(type));

    for (const auto& item: items)
        if (item.type == type)
            out.push_back(item.hash);

    return out;
}

size_t inventory::count(type_id type) const
{
    const auto is_type = [type](const inventory_item& item)
    {
        return item.type == type;
    };

    return std::count_if(items.begin(), items.end(), is_type);
}

} // namespace messages
} // namespace network
} // namespace libbitcoin
