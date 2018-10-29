#pragma once

#include "charls_error.h"
#include <charls/charls.h>

#include <vector>
#include <cstddef>

//#include "span.h"

namespace charls {

struct metadata_info_t
{
    int width;
    int height;
    int bits_per_sample;
    int component_count;
};


class decoder
{
public:
    decoder() = default;

    void read_header(const void* source, const size_t source_size_bytes)
    {
        char error_message[ErrorMessageSize];

        const auto result = JpegLsReadHeader(source, source_size_bytes, &params_, error_message);
        if (result != ApiResult::OK)
            throw charls_error(result, error_message);

        source_ = source;
        source_size_bytes_ = source_size_bytes;
        metadata_ = { params_.width, params_.height, params_.bitsPerSample, params_.components };
    }

    const metadata_info_t& metadata_info() const
    {
        return metadata_;
    }

    std::vector<std::byte> decode() const
    {
        std::vector<std::byte> buffer(params_.width * params_.height * params_.components * (params_.bitsPerSample <= 8 ? 1 : 2));

        decode(buffer.data(), buffer.size());

        return buffer;
    }

    void decode(void* destination, const size_t destination_size_bytes) const
    {
        char error_message[ErrorMessageSize];

        const auto result = JpegLsDecode(destination, destination_size_bytes, source_, source_size_bytes_, &params_, error_message);
        if (result != ApiResult::OK)
            throw charls_error(result, error_message);
    }

private:
    const void* source_{};
    size_t source_size_bytes_{};
    JlsParameters params_{};
    metadata_info_t metadata_{};
};

}
