/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "public/platform/WebThreadSafeData.h"

#include "platform/blob/BlobData.h"

namespace blink {

WebThreadSafeData::WebThreadSafeData(const char* data, size_t length) {
  m_private = RawData::create();
  m_private->mutableData()->append(data, length);
}

void WebThreadSafeData::reset() {
  m_private.reset();
}

void WebThreadSafeData::assign(const WebThreadSafeData& other) {
  m_private = other.m_private;
}

size_t WebThreadSafeData::size() const {
  if (m_private.isNull())
    return 0;
  return m_private->length();
}

const char* WebThreadSafeData::data() const {
  if (m_private.isNull())
    return 0;
  return m_private->data();
}

WebThreadSafeData::WebThreadSafeData(PassRefPtr<RawData> data)
    : m_private(data) {}

WebThreadSafeData::WebThreadSafeData(const WebThreadSafeData& other) {
  m_private = other.m_private;
}

WebThreadSafeData& WebThreadSafeData::operator=(
    const WebThreadSafeData& other) {
  m_private = other.m_private;
  return *this;
}

WebThreadSafeData& WebThreadSafeData::operator=(PassRefPtr<RawData> data) {
  m_private = data;
  return *this;
}

}  // namespace blink
