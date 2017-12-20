// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_P2P_SOCKET_CLIENT_H_
#define CONTENT_RENDERER_P2P_SOCKET_CLIENT_H_

#include <stdint.h>

#include <vector>

#include "base/memory/ref_counted.h"
#include "content/common/p2p_socket_type.h"
#include "net/base/ip_endpoint.h"

namespace rtc {
struct PacketOptions;
};

namespace content {

class P2PSocketClientDelegate;

// P2P socket that routes all calls over IPC.
// Note that while ref-counting is thread-safe, all methods must be
// called on the same thread.
class P2PSocketClient : public base::RefCountedThreadSafe<P2PSocketClient> {
 public:
  // Create a new P2PSocketClient() of the specified |type| and connected to
  // the specified |address|. |address| matters only when |type| is set to
  // P2P_SOCKET_TCP_CLIENT. The methods on the returned socket may only be
  // called on the same thread that created it.
  static scoped_refptr<P2PSocketClient> Create(
      P2PSocketType type,
      const net::IPEndPoint& local_address,
      uint16_t min_port,
      uint16_t max_port,
      const net::IPEndPoint& remote_address,
      P2PSocketClientDelegate* delegate);

  P2PSocketClient() {}

  // Send the |data| to the |address| using Differentiated Services Code Point
  // |dscp|. Return value is the unique packet_id for this packet.
  virtual uint64_t Send(const net::IPEndPoint& address,
                        const std::vector<char>& data,
                        const rtc::PacketOptions& options) = 0;

  virtual void SetOption(P2PSocketOption option, int value) = 0;

  // Must be called before the socket is destroyed.
  virtual void Close() = 0;

  virtual int GetSocketID() const = 0;
  virtual void SetDelegate(P2PSocketClientDelegate* delegate) = 0;

 protected:
  virtual ~P2PSocketClient() {}

 private:
  // Calls destructor.
  friend class base::RefCountedThreadSafe<P2PSocketClient>;
};
}  // namespace content

#endif  // CONTENT_RENDERER_P2P_SOCKET_CLIENT_H_
