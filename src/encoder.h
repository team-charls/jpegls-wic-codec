#pragma once

#include "charls.h"

#include <vector>
#include <cstddef>

namespace charls {

struct metadata_info_t
{
    int width;
    int height;
    int bits_per_component;
    int component_count;
};


class encoder
{
public:
    int allowed_lossy_error{};
    metadata_info_t metadata_info{};

    encoder() = default;

    std::vector<std::byte> encode(const void* source, size_t source_size_bytes)
    {
    }

    size_t encode(const void* source, size_t source_size_bytes, void* destination, size_t destination_size_bytes)
    {
        size_t bytes_written;
        JlsParameters parameters;
        char error_message[ErrorMessageSize];

        auto result = JpegLsEncode(destination, destination_size_bytes, &bytes_written,
            source, source_size_bytes, &parameters, error_message);
        if (result != ApiResult::OK)
            throw charls_error(result, error_message);

        return bytes_written;
    }
};

} // namespace charls
