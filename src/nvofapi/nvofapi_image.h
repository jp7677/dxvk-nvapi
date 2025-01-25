/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "../nvofapi_private.h"

namespace dxvk {
    class NvOFImage {
      public:
        NvOFImage(VkDevice device, VkImage image, VkFormat format) : m_vkDevice(device), m_image(image), m_format(format) {
        }

        ~NvOFImage() {
            m_vkDestroyImageView(m_vkDevice, m_imageView, nullptr);
        }

        [[nodiscard]] VkImageView ImageView() const { return m_imageView; }

        bool Initialize(PFN_vkCreateImageView CreateImageView,
            PFN_vkDestroyImageView DestroyImageView);

      private:
        VkDevice m_vkDevice{};
        VkImage m_image{};
        VkImageView m_imageView{};
        VkFormat m_format{};
        PFN_vkDestroyImageView m_vkDestroyImageView{};
    };
}
