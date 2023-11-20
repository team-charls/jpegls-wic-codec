// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

module;

#include <memory>
#include <cstddef>

export module storage_buffer;

// purpose: replacement container for std::vector that doesn't initialize its content
export class storage_buffer final
{
public:
    using value_type = std::byte;

    explicit storage_buffer(const size_t size) :
        size_{size}, buffer_{std::make_unique_for_overwrite<std::byte[]>(size)} // NOLINT(cppcoreguidelines-avoid-c-arrays)
    {
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] std::byte* data() const noexcept
    {
        return buffer_.get();
    }

private:
    size_t size_;
    std::unique_ptr<std::byte[]> buffer_; // NOLINT(cppcoreguidelines-avoid-c-arrays)
};

