// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

export module portable_anymap_file;

import "std.h";


[[nodiscard]]
constexpr int32_t log_2(const int32_t n) noexcept
{
    int32_t x{};
    while (n > (1 << x))
    {
        ++x;
    }
    return x;
}


// Purpose: this class can read an image stored in the Portable Anymap Format (PNM).
//          The 2 binary formats P5 and P6 are supported:
//          Portable GrayMap: P5 = binary, extension = .pgm, 0-2^16 (gray scale)
//          Portable PixMap: P6 = binary, extension.ppm, range 0-2^16 (RGB)
export class portable_anymap_file final
{
public:
    /// <exception cref="ifstream::failure">Thrown when the input file cannot be read.</exception>
    explicit portable_anymap_file(const char* filename)
    {
        std::ifstream pnm_file;
        pnm_file.exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);
        pnm_file.open(filename, std::ios_base::in | std::ios_base::binary);

        const std::vector header_info{read_header(pnm_file)};
        if (header_info.size() != 4)
            throw std::ios_base::failure("Incorrect PNM header");

        component_count_ = header_info[0] == 6 ? 3 : 1;
        width_ = header_info[1];
        height_ = header_info[2];
        bits_per_sample_ = log_2(header_info[3] + 1);

        const int bytes_per_sample{(bits_per_sample_ + 7) / 8};
        input_buffer_.resize(static_cast<size_t>(width_) * height_ * bytes_per_sample * component_count_);
        pnm_file.read(reinterpret_cast<char*>(input_buffer_.data()), input_buffer_.size());

        convert_to_little_endian_if_needed();
    }

    [[nodiscard]]
    int width() const noexcept
    {
        return width_;
    }

    [[nodiscard]]
    int height() const noexcept
    {
        return height_;
    }

    [[nodiscard]]
    int component_count() const noexcept
    {
        return component_count_;
    }

    [[nodiscard]]
    int bits_per_sample() const noexcept
    {
        return bits_per_sample_;
    }

    [[nodiscard]]
    std::vector<std::byte>& image_data() noexcept
    {
        return input_buffer_;
    }

    [[nodiscard]]
    std::span<std::uint16_t> image_data_as_uint16() noexcept
    {
        void* data{input_buffer_.data()};
        return std::span{static_cast<std::uint16_t*>(data), input_buffer_.size() / 2};
    }

private:
    [[nodiscard]]
    static std::vector<int> read_header(std::istream& pnm_file)
    {
        // All portable anymap format (PNM) start with the character P.
        if (const auto first{static_cast<char>(pnm_file.get())}; first != 'P')
            throw std::istream::failure("Missing P");

        std::vector<int> result;
        while (result.size() < 4)
        {
            std::string bytes;
            std::getline(pnm_file, bytes);
            std::stringstream line(bytes);

            while (result.size() < 4)
            {
                int value{-1};
                line >> value;
                if (value <= 0)
                    break;

                result.push_back(value);
            }
        }
        return result;
    }

    void convert_to_little_endian_if_needed() noexcept
    {
        // Anymap files with multi byte pixels are stored in big endian format in the file.
        if (bits_per_sample_ > 8)
        {
            for (size_t i{}; i < input_buffer_.size() - 1; i += 2)
            {
                std::swap(input_buffer_[i], input_buffer_[i + 1]);
            }
        }
    }

    int component_count_;
    int width_;
    int height_;
    int bits_per_sample_;
    std::vector<std::byte> input_buffer_;
};
