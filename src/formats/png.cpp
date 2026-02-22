#include <api.hpp>
#include <spng.h>

#include "../FakeVector.hpp"

using namespace geode;

IMAGE_PLUS_BEGIN_NAMESPACE
namespace decode {
    static constexpr bool hasAlpha = true;

    static Result<> parseHeader(void const* data, size_t size, spng_ctx* ctx, spng_ihdr& ihdr) {
        if (!ctx || spng_set_png_buffer(ctx, data, size) != 0)
            return Err("Failed to create PNG context or set buffer");

        if (spng_get_ihdr(ctx, &ihdr) != 0)
            return Err("Failed to get PNG header");

        if (ihdr.width > 65535 || ihdr.height > 65535)
            return Err("PNG image dimensions exceed 65535 pixels");

        return Ok();
    }

    Result<DecodedImage> png(void const* data, size_t size) {
        std::unique_ptr<spng_ctx, decltype(&spng_ctx_free)> ctx(spng_ctx_new(0), &spng_ctx_free);
        spng_ihdr ihdr;
        GEODE_UNWRAP(parseHeader(data, size, ctx.get(), ihdr));

        auto fmt = hasAlpha ? SPNG_FMT_RGBA8 : SPNG_FMT_RGB8;

        size_t totalSize;
        if (spng_decoded_image_size(ctx.get(), fmt, &totalSize) != 0)
            return Err("Failed to get PNG decoded image size");

        auto output = std::make_unique<uint8_t[]>(totalSize);
        if (!output)
            return Err("Failed to allocate memory for PNG image data");

        if (spng_decode_image(ctx.get(), output.get(), totalSize, fmt, hasAlpha ? SPNG_DECODE_TRNS : 0) != 0) {
            return Err("Failed to decode PNG image");
        }

        return Ok(DecodedImage{
            .data = std::move(output),
            .width = static_cast<uint16_t>(ihdr.width),
            .height = static_cast<uint16_t>(ihdr.height),
            .bit_depth = ihdr.bit_depth,
            .hasAlpha = hasAlpha
        });
    }

    Result<DecodedImage> pngHeader(void const* data, size_t size) {
        std::unique_ptr<spng_ctx, decltype(&spng_ctx_free)> ctx(spng_ctx_new(0), &spng_ctx_free);
        spng_ihdr ihdr;
        GEODE_UNWRAP(parseHeader(data, size, ctx.get(), ihdr));

        return Ok(DecodedImage {
            .data = nullptr,
            .width = static_cast<uint16_t>(ihdr.width),
            .height = static_cast<uint16_t>(ihdr.height),
            .bit_depth = ihdr.bit_depth,
            .hasAlpha = hasAlpha
        });
    }

    Result<size_t> pngInto(void const* data, size_t size, void* buf, size_t bufSize) {
        std::unique_ptr<spng_ctx, decltype(&spng_ctx_free)> ctx(spng_ctx_new(0), &spng_ctx_free);
        spng_ihdr ihdr;
        GEODE_UNWRAP(parseHeader(data, size, ctx.get(), ihdr));

        auto fmt = hasAlpha ? SPNG_FMT_RGBA8 : SPNG_FMT_RGB8;

        size_t totalSize;
        if (spng_decoded_image_size(ctx.get(), fmt, &totalSize) != 0)
            return Err("Failed to get PNG decoded image size");

        if (bufSize < totalSize)
            return Err("Output buffer is too small for decoded PNG image");

        if (spng_decode_image(ctx.get(), buf, bufSize, fmt, hasAlpha ? SPNG_DECODE_TRNS : 0) != 0) {
            return Err("Failed to decode PNG image");
        }

        return Ok(totalSize);
    }
}

namespace encode {
    Result<ByteVector> png(void const* image, uint16_t width, uint16_t height, bool hasAlpha) {
        if (!image)
            return Err("Invalid image data");

        std::unique_ptr<spng_ctx, decltype(&spng_ctx_free)> ctx(spng_ctx_new(SPNG_CTX_ENCODER), &spng_ctx_free);
        if (!ctx)
            return Err("Failed to create PNG context");

        // Enable encoding to internal buffer
        if (spng_set_option(ctx.get(), SPNG_ENCODE_TO_BUFFER, 1) != 0)
            return Err("Failed to set buffer encoding option");

        spng_ihdr ihdr = {};
        ihdr.width = width;
        ihdr.height = height;
        ihdr.bit_depth = 8;
        ihdr.color_type = hasAlpha ? SPNG_COLOR_TYPE_TRUECOLOR_ALPHA : SPNG_COLOR_TYPE_TRUECOLOR;

        if (spng_set_ihdr(ctx.get(), &ihdr) != 0)
            return Err("Failed to set PNG header");

        // Calculate the expected data size
        size_t channels = hasAlpha ? 4 : 3;
        size_t expectedSize = static_cast<size_t>(width) * height * channels;

        // Use SPNG_FMT_PNG to match the format specified in ihdr
        if (spng_encode_image(ctx.get(), image, expectedSize, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE) != 0)
            return Err("Failed to encode PNG image");

        // Get the encoded PNG buffer
        size_t pngSize;
        int ret;
        void* pngBuffer = spng_get_png_buffer(ctx.get(), &pngSize, &ret);

        if (!pngBuffer || ret != 0)
            return Err("Failed to get PNG buffer");

        return Ok(FakeVector(pngBuffer, pngSize));
    }
}
IMAGE_PLUS_END_NAMESPACE
