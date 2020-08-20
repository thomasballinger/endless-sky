/* ImageBuffer.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "ImageBuffer.h"

#include "File.h"

#include <png.h>
#include <jpeglib.h>
#include <webp/decode.h>
#include <webp/demux.h>
#include <strings.h>
#include <string.h>
#include <memory>

#include <cstdio>
#include <vector>

using namespace std;

namespace {
	bool ReadPNG(const string &path, ImageBuffer &buffer, uint32_t frame);
	bool ReadJPG(const string &path, ImageBuffer &buffer, uint32_t frame);
	bool ReadWEBP(const string &path, ImageBuffer &buffer, uint32_t frame);
	void Premultiply(ImageBuffer &buffer, uint32_t frame, int additive);
	int Compare(const char *a, const char *b, bool caseSensitive)
	{
		if (a == nullptr || b == nullptr)
			return false;

		if (caseSensitive)
		{
			return strcmp(a, b);
		}
		else
		{
			return strcasecmp(a, b);
		}
	}
	bool EndsWith(const std::string &str, const std::string &match, bool caseSensitive = true)
	{
		if (str.size() >= match.size())
		{
			auto start = str.c_str() + str.size() - match.size();
			return Compare(start, match.c_str(), caseSensitive) == 0;
		}
		return false;
	}
}


ImageBuffer::ImageBuffer(uint32_t frames)
	: width(0), height(0), frames(frames), pixels(nullptr)
{
}



ImageBuffer::~ImageBuffer()
{
	Clear();
}



// Set the number of frames. This must be called before allocating.
void ImageBuffer::Clear(uint32_t frames)
{
	delete [] pixels;
	pixels = nullptr;
	this->frames = frames;
}



// Allocate the internal buffer. This must only be called once for each
// image buffer; subsequent calls will be ignored.
void ImageBuffer::Allocate(uint32_t width, uint32_t height)
{
	// Do nothing if the buffer is already allocated or if any of the dimensions
	// is set to zero.
	if(pixels || !width || !height || !frames)
		return;
	
	this->width = width;
	this->height = height;
	pixels = new uint32_t[width * height * frames];
}



uint32_t ImageBuffer::Width() const
{
	return width;
}



uint32_t ImageBuffer::Height() const
{
	return height;
}



uint32_t ImageBuffer::Frames() const
{
	return frames;
}



const uint32_t *ImageBuffer::Pixels() const
{
	return pixels;
}



uint32_t *ImageBuffer::Pixels()
{
	return pixels;
}



const uint32_t *ImageBuffer::Begin(uint32_t y, uint32_t frame) const
{
	return pixels + width * (y + height * frame);
}



uint32_t *ImageBuffer::Begin(uint32_t y, uint32_t frame)
{
	return pixels + width * (y + height * frame);
}



void ImageBuffer::ShrinkToHalfSize()
{
	ImageBuffer result(frames);
	result.Allocate(width / 2, height / 2);
	
	unsigned char *begin = reinterpret_cast<unsigned char *>(pixels);
	unsigned char *out = reinterpret_cast<unsigned char *>(result.pixels);
	// Loop through every line of every frame of the buffer.
	for(uint32_t y = 0; y < result.height * frames; ++y)
	{
		unsigned char *aIt = begin + (4 * width) * (2 * y);
		unsigned char *aEnd = aIt + 4 * 2 * result.width;
		unsigned char *bIt = begin + (4 * width) * (2 * y + 1);
		for( ; aIt != aEnd; aIt += 4, bIt += 4)
		{
			for(int channel = 0; channel < 4; ++channel, ++aIt, ++bIt, ++out)
				*out = (static_cast<unsigned>(aIt[0]) + static_cast<unsigned>(bIt[0])
					+ static_cast<unsigned>(aIt[4]) + static_cast<unsigned>(bIt[4]) + 2) / 4;
		}
	}
	swap(width, result.width);
	swap(height, result.height);
	swap(pixels, result.pixels);
}



bool ImageBuffer::Read(const string &path, uint32_t frame)
{
	// First, make sure this is a JPG or PNG file.
	if(path.length() < 4)
		return false;
	
	bool isPNG = EndsWith(path, ".png", false);
	bool isJPG = EndsWith(path, ".jpg", false);
	bool isWEBP = EndsWith(path, ".webp", false);
	if(!isPNG && !isJPG && !isWEBP)
	{
		printf("%s is invalid\n", path.c_str());
		return false;
	}

	if(isPNG && !ReadPNG(path, *this, frame))
		return false;
	if(isJPG && !ReadJPG(path, *this, frame))
		return false;
	if(isWEBP && !ReadWEBP(path, *this, frame))
		return false;

	// Check if the sprite uses additive blending. Start by getting the index of
	// the last character before the frame number (if one is specified).
	int pos = path.rfind(".");
	if(pos > 3 && !path.compare(pos - 3, 3, "@2x"))
		pos -= 3;
	while(--pos)
		if(path[pos] < '0' || path[pos] > '9')
			break;
	// Special case: if the image is already in premultiplied alpha format,
	// there is no need to apply premultiplication here.
	if(path[pos] != '=')
	{
		int additive = (path[pos] == '+') ? 2 : (path[pos] == '~') ? 1 : 0;
		if(isPNG || isWEBP || (isJPG && additive == 2))
			Premultiply(*this, frame, additive);
	}
	return true;
}



namespace {
	bool ReadPNG(const string &path, ImageBuffer &buffer, uint32_t frame)
	{
		// Open the file, and make sure it really is a PNG.
		File file(path);
		if(!file)
			return false;
		
		// Set up libpng.
		png_struct *png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if(!png)
			return false;
		
		png_info *info = png_create_info_struct(png);
		if(!info)
		{
			png_destroy_read_struct(&png, nullptr, nullptr);
			return false;
		}
		
		if(setjmp(png_jmpbuf(png)))
		{
			png_destroy_read_struct(&png, &info, nullptr);
			return false;
		}
		
		png_init_io(png, file);
		png_set_sig_bytes(png, 0);
		
		png_read_info(png, info);
		uint32_t width = png_get_image_width(png, info);
		uint32_t height = png_get_image_height(png, info);
		// If the buffer is not yet allocated, allocate it.
		buffer.Allocate(width, height);
		// Make sure this frame's dimensions are valid.
		if(!width || !height || width != buffer.Width() || height != buffer.Height())
		{
			png_destroy_read_struct(&png, &info, nullptr);
			return false;
		}
		
		// Adjust settings to make sure the result will be a BGRA file.
		int colorType = png_get_color_type(png, info);
		int bitDepth = png_get_bit_depth(png, info);
		
		png_set_strip_16(png);
		png_set_packing(png);
		if(colorType == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png);
		if(colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
			png_set_expand_gray_1_2_4_to_8(png);
		if(colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
			png_set_gray_to_rgb(png);
		// Let libpng handle any interlaced image decoding.
		png_set_interlace_handling(png);
		png_read_update_info(png, info);
		
		// Read the file.
		vector<png_byte *> rows(height, nullptr);
		for(uint32_t y = 0; y < height; ++y)
			rows[y] = reinterpret_cast<png_byte *>(buffer.Begin(y, frame));
		
		png_read_image(png, &rows.front());
		
		// Clean up. The file will be closed automatically.
		png_destroy_read_struct(&png, &info, nullptr);
		
		return true;
	}
	
	
	
	bool ReadJPG(const string &path, ImageBuffer &buffer, uint32_t frame)
	{
		File file(path);
		if(!file)
			return false;
		
		jpeg_decompress_struct cinfo;
		struct jpeg_error_mgr jerr;
		cinfo.err = jpeg_std_error(&jerr);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
		jpeg_create_decompress(&cinfo);
#pragma GCC diagnostic pop
		
		jpeg_stdio_src(&cinfo, file);
		jpeg_read_header(&cinfo, TRUE);
		cinfo.out_color_space = JCS_RGB;
		
		jpeg_start_decompress(&cinfo);
		uint32_t width = cinfo.image_width;
		uint32_t height = cinfo.image_height;
		// If the buffer is not yet allocated, allocate it.
		buffer.Allocate(width, height);
		// Make sure this frame's dimensions are valid.
		if(!width || !height || width != buffer.Width() || height != buffer.Height())
		{
			jpeg_finish_decompress(&cinfo);
			jpeg_destroy_decompress(&cinfo);
			return false;
		}
		
		// Read the file.
		vector<JSAMPLE *> rows(height, nullptr);
		for(uint32_t y = 0; y < height; ++y)
			rows[y] = reinterpret_cast<JSAMPLE *>(buffer.Begin(y, frame));
		
		while(height)
			height -= jpeg_read_scanlines(&cinfo, &rows.front() + cinfo.output_scanline, height);
		
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		
		// Expand RGB to RGBA
		for (unsigned int y = 0; y < cinfo.image_height; y++)
		{
			auto pptr = reinterpret_cast<uint8_t*>(buffer.Begin(y, frame));
			for (int idx = width - 1; idx >= 0; --idx)
			// for(int idx = 0; idx < width; ++idx)
			{
				pptr[idx * 4 + 0] = pptr[idx * 3 + 0];
				pptr[idx * 4 + 1] = pptr[idx * 3 + 1];
				pptr[idx * 4 + 2] = pptr[idx * 3 + 2];
				pptr[idx * 4 + 3] = static_cast<unsigned char>(0xFF);
			}
		}

		return true;
	}
	
	bool ReadWEBP(const string &path, ImageBuffer &buffer, uint32_t frame)
	{
		printf("Trying to read webp image %s\n", path.c_str());
		File file(path);
		if (!file)
			return false;

		FILE* f = file;
		fseek(f, 0, SEEK_END);
		size_t file_size = ftell(f);
		fseek(f, 0, SEEK_SET);
		auto tmpbuf = std::vector<uint8_t>(file_size);
		auto elems_read = fread(tmpbuf.data(), file_size, 1, f);
		if (elems_read != 1)
		{
			return false;
		}
		// WebPMux* WebPMuxCreate(const WebPData* bitstream, int copy_data);
		WebPData bitstream = {
			.bytes = tmpbuf.data(),
			.size = file_size,
		};

		auto webp_deleter = [](WebPDemuxer* demux) { WebPDemuxDelete(demux); };
		std::unique_ptr<WebPDemuxer, decltype(webp_deleter)> demux(WebPDemux(&bitstream), webp_deleter);
		uint32_t width = WebPDemuxGetI(demux.get(), WEBP_FF_CANVAS_WIDTH);
		uint32_t height = WebPDemuxGetI(demux.get(), WEBP_FF_CANVAS_HEIGHT);
		uint32_t flags = WebPDemuxGetI(demux.get(), WEBP_FF_FORMAT_FLAGS);
		uint32_t frame_count = WebPDemuxGetI(demux.get(), WEBP_FF_FRAME_COUNT);
		printf("The image has %u frames and is %ux%u sized\n", frame_count, width, height);
		if (frame_count > 1) {
			buffer.Clear(frame_count);
		}

		buffer.Allocate(width, height);
		// Make sure this frame's dimensions are valid.
		if (!width || !height || width != buffer.Width() || height != buffer.Height())
		{
			return false;
		}

		WebPIterator iter;
		if (WebPDemuxGetFrame(demux.get(), 1, &iter))
		{
			auto iter_deleter_lambda = [](WebPIterator* iter) { WebPDemuxReleaseIterator(iter); };
			std::unique_ptr<WebPIterator, decltype(iter_deleter_lambda)> iter_deleter(&iter, iter_deleter_lambda);
			do
			{
				printf("Decoding frame %d out of %d\n", iter.frame_num, frame_count);
				auto decoded = WebPDecodeRGBAInto(
					iter.fragment.bytes, iter.fragment.size, (uint8_t*)buffer.Begin(0, iter.frame_num - 1), width * height * 4, width * 4);

				if (decoded == nullptr)
				{
					printf("Bad stuff happened!\n");
					return false;
				}
			} while (WebPDemuxNextFrame(&iter));
		}

		return true;
	}
	
	void Premultiply(ImageBuffer &buffer, uint32_t frame, int additive)
	{
		for(uint32_t y = 0; y < buffer.Height(); ++y)
		{
			uint32_t *it = buffer.Begin(y, frame);
			
			for(uint32_t *end = it + buffer.Width(); it != end; ++it)
			{
				uint64_t value = *it;
				uint64_t alpha = (value & 0xFF000000) >> 24;
				
				uint64_t red = (((value & 0xFF0000) * alpha) / 255) & 0xFF0000;
				uint64_t green = (((value & 0xFF00) * alpha) / 255) & 0xFF00;
				uint64_t blue = (((value & 0xFF) * alpha) / 255) & 0xFF;
				
				value = red | green | blue;
				if(additive == 1)
					alpha >>= 2;
				if(additive != 2)
					value |= (alpha << 24);
				
				*it = static_cast<uint32_t>(value);
			}
		}
	}
}
