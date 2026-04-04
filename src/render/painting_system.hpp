#pragma once

/**
 * PAINTING SYSTEM — Image Loading and GPU Management
 * ===================================================
 *
 * Reusable module for loading images into a texture array.
 * Handles aspect ratio preservation by centering images within
 * a standard canvas size.
 *
 * USAGE:
 *   PaintingTextureManager textures;
 *   textures.init(device, MAX_PAINTINGS);
 *   
 *   auto [layer, uv_x, uv_y] = textures.load_image("painting.png");
 *   // Use layer as texture_layer, uv_x/uv_y as uv_scale in GPUPainting
 *   
 *   textures.finalize(queue);  // Upload all to GPU
 *
 * DEPENDENCIES:
 *   - stb_image.h (include in ONE .cpp file with STB_IMAGE_IMPLEMENTATION)
 */

#include <webgpu/webgpu_cpp.h>
#include <vector>
#include <string>
#include <array>
#include <cstring>
#include <iostream>

// Forward declare stb_image functions (user must include stb_image.h somewhere)
extern "C" {
    unsigned char* stbi_load(const char* filename, int* x, int* y, int* channels, int desired_channels);
    void stbi_image_free(void* retval_from_stbi_load);
}

namespace t7 {
namespace painting {


// ═══════════════════════════════════════════════════════════════════════════════
// §1 IMAGE DATA — Intermediate storage before GPU upload
// ═══════════════════════════════════════════════════════════════════════════════

struct LoadedImage {
    std::vector<uint8_t> pixels;  // RGBA8
    uint32_t width = 0;
    uint32_t height = 0;
    float uv_scale_x = 1.0f;      // How much of the canvas the image occupies
    float uv_scale_y = 1.0f;
    bool valid = false;
};


// ═══════════════════════════════════════════════════════════════════════════════
// §2 TEXTURE MANAGER — Handles loading and GPU texture array
// ═══════════════════════════════════════════════════════════════════════════════

class PaintingTextureManager {
public:
    static constexpr uint32_t CANVAS_SIZE = 2048;

    bool init(wgpu::Device device, uint32_t max_layers) {
        device_ = device;
        maxLayers_ = max_layers;
        images_.reserve(max_layers);
        
        // Create texture array
        wgpu::TextureDescriptor desc{};
        desc.label = "Painting Array";
        desc.size = { CANVAS_SIZE, CANVAS_SIZE, max_layers };
        desc.format = wgpu::TextureFormat::RGBA8Unorm;
        desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.mipLevelCount = 1;
        desc.sampleCount = 1;
        
        textureArray_ = device_.CreateTexture(&desc);
        if (!textureArray_) {
            std::cerr << "[PaintingSystem] Failed to create texture array\n";
            return false;
        }
        
        // Create view for the entire array
        wgpu::TextureViewDescriptor viewDesc{};
        viewDesc.label = "Painting Array View";
        viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
        viewDesc.dimension = wgpu::TextureViewDimension::e2DArray;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = max_layers;
        
        textureView_ = textureArray_.CreateView(&viewDesc);
        if (!textureView_) {
            std::cerr << "[PaintingSystem] Failed to create texture view\n";
            return false;
        }
        
        // Create sampler
        wgpu::SamplerDescriptor samplerDesc{};
        samplerDesc.label = "Painting Sampler";
        samplerDesc.addressModeU = wgpu::AddressMode::ClampToEdge;
        samplerDesc.addressModeV = wgpu::AddressMode::ClampToEdge;
        samplerDesc.addressModeW = wgpu::AddressMode::ClampToEdge;
        samplerDesc.magFilter = wgpu::FilterMode::Linear;
        samplerDesc.minFilter = wgpu::FilterMode::Linear;
        samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;
        
        sampler_ = device_.CreateSampler(&samplerDesc);
        if (!sampler_) {
            std::cerr << "[PaintingSystem] Failed to create sampler\n";
            return false;
        }
        
        std::cout << "[PaintingSystem] Initialized with " << max_layers 
                  << " layers at " << CANVAS_SIZE << "x" << CANVAS_SIZE << "\n";
        return true;
    }

    // Load an image from file, returns layer index and UV scale
    // Returns {layer, uv_scale_x, uv_scale_y} or {UINT32_MAX, 0, 0} on failure
    struct LoadResult {
        uint32_t layer;
        float uv_scale_x;
        float uv_scale_y;
    };

    LoadResult load_image(const std::string& path) {
        if (images_.size() >= maxLayers_) {
            std::cerr << "[PaintingSystem] Maximum layers reached\n";
            return { UINT32_MAX, 0.0f, 0.0f };
        }
        
        int w, h, channels;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);  // Force RGBA
        
        if (!data) {
            std::cerr << "[PaintingSystem] Failed to load: " << path << "\n";
            return { UINT32_MAX, 0.0f, 0.0f };
        }
        
        uint32_t layer = static_cast<uint32_t>(images_.size());
        images_.emplace_back();
        LoadedImage& img = images_.back();
        
        img.width = static_cast<uint32_t>(w);
        img.height = static_cast<uint32_t>(h);
        img.valid = true;
        
        // Compute scaling to fit within canvas while preserving aspect ratio
        float scale_x = static_cast<float>(CANVAS_SIZE) / w;
        float scale_y = static_cast<float>(CANVAS_SIZE) / h;
        float scale = std::min(scale_x, scale_y);
        
        uint32_t scaled_w = static_cast<uint32_t>(w * scale);
        uint32_t scaled_h = static_cast<uint32_t>(h * scale);
        
        // UV scale is how much of the canvas the image occupies
        img.uv_scale_x = static_cast<float>(scaled_w) / CANVAS_SIZE;
        img.uv_scale_y = static_cast<float>(scaled_h) / CANVAS_SIZE;
        
        // Create canvas with image centered
        img.pixels.resize(CANVAS_SIZE * CANVAS_SIZE * 4, 0);  // Black background
        
        // Simple nearest-neighbor scaling and centering
        uint32_t offset_x = (CANVAS_SIZE - scaled_w) / 2;
        uint32_t offset_y = (CANVAS_SIZE - scaled_h) / 2;
        
        for (uint32_t cy = 0; cy < scaled_h; ++cy) {
            for (uint32_t cx = 0; cx < scaled_w; ++cx) {
                // Source pixel (nearest neighbor)
                uint32_t sx = static_cast<uint32_t>((cx / scale));
                uint32_t sy = static_cast<uint32_t>((cy / scale));
                sx = std::min(sx, static_cast<uint32_t>(w - 1));
                sy = std::min(sy, static_cast<uint32_t>(h - 1));
                
                uint32_t src_idx = (sy * w + sx) * 4;
                uint32_t dst_idx = ((offset_y + cy) * CANVAS_SIZE + (offset_x + cx)) * 4;
                
                img.pixels[dst_idx + 0] = data[src_idx + 0];
                img.pixels[dst_idx + 1] = data[src_idx + 1];
                img.pixels[dst_idx + 2] = data[src_idx + 2];
                img.pixels[dst_idx + 3] = data[src_idx + 3];
            }
        }
        
        stbi_image_free(data);
        
        std::cout << "[PaintingSystem] Loaded layer " << layer << ": " << path 
                  << " (" << w << "x" << h << " -> " << scaled_w << "x" << scaled_h 
                  << ", uv=" << img.uv_scale_x << "," << img.uv_scale_y << ")\n";
        
        return { layer, img.uv_scale_x, img.uv_scale_y };
    }

    // Create a solid color layer (for testing or placeholder)
    LoadResult create_solid(float r, float g, float b) {
        if (images_.size() >= maxLayers_) {
            return { UINT32_MAX, 0.0f, 0.0f };
        }
        
        uint32_t layer = static_cast<uint32_t>(images_.size());
        images_.emplace_back();
        LoadedImage& img = images_.back();
        
        img.width = CANVAS_SIZE;
        img.height = CANVAS_SIZE;
        img.uv_scale_x = 1.0f;
        img.uv_scale_y = 1.0f;
        img.valid = true;
        
        uint8_t rb = static_cast<uint8_t>(r * 255);
        uint8_t gb = static_cast<uint8_t>(g * 255);
        uint8_t bb = static_cast<uint8_t>(b * 255);
        
        img.pixels.resize(CANVAS_SIZE * CANVAS_SIZE * 4);
        for (uint32_t i = 0; i < CANVAS_SIZE * CANVAS_SIZE; ++i) {
            img.pixels[i * 4 + 0] = rb;
            img.pixels[i * 4 + 1] = gb;
            img.pixels[i * 4 + 2] = bb;
            img.pixels[i * 4 + 3] = 255;
        }
        
        return { layer, 1.0f, 1.0f };
    }

    // Upload all loaded images to GPU
    void finalize(wgpu::Queue& queue) {
        for (uint32_t i = 0; i < images_.size(); ++i) {
            if (!images_[i].valid) continue;
            
            wgpu::TexelCopyBufferLayout layout{};
            layout.offset = 0;
            layout.bytesPerRow = CANVAS_SIZE * 4;
            layout.rowsPerImage = CANVAS_SIZE;
            
            wgpu::TexelCopyTextureInfo dest{};
            dest.texture = textureArray_;
            dest.mipLevel = 0;
            dest.origin = { 0, 0, i };
            dest.aspect = wgpu::TextureAspect::All;
            
            wgpu::Extent3D extent = { CANVAS_SIZE, CANVAS_SIZE, 1 };
            
            queue.WriteTexture(&dest, images_[i].pixels.data(), 
                              images_[i].pixels.size(), &layout, &extent);
        }
        
        std::cout << "[PaintingSystem] Uploaded " << images_.size() << " layers to GPU\n";
    }

    // Accessors for bind group creation
    wgpu::TextureView texture_view() const { return textureView_; }
    wgpu::Sampler sampler() const { return sampler_; }
    uint32_t layer_count() const { return static_cast<uint32_t>(images_.size()); }

    // Get UV scale for a loaded layer
    std::pair<float, float> get_uv_scale(uint32_t layer) const {
        if (layer < images_.size()) {
            return { images_[layer].uv_scale_x, images_[layer].uv_scale_y };
        }
        return { 1.0f, 1.0f };
    }

private:
    wgpu::Device device_;
    wgpu::Texture textureArray_;
    wgpu::TextureView textureView_;
    wgpu::Sampler sampler_;
    
    std::vector<LoadedImage> images_;
    uint32_t maxLayers_ = 0;
};


} // namespace painting
} // namespace t7
