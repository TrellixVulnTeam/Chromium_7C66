// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_INSTALLER_MAC_APP_OMAHAXMLPARSER_H_
#define CHROME_INSTALLER_MAC_APP_OMAHAXMLPARSER_H_

#import <Foundation/Foundation.h>

@interface OmahaXMLParser : NSObject

// Parses the XML body from Omaha's HTTP response and extracts the URLs and name
// of the Chrome disk image. Then, returns an array with all the URLs
// concatenated with the filename.
+ (NSArray*)parseXML:(NSData*)omahaResponseXML error:(NSError**)error;

@end

#endif  // CHROME_INSTALLER_MAC_APP_OMAHAXMLPARSER_H_
