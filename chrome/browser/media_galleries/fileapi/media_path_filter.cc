// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media_galleries/fileapi/media_path_filter.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

#include <algorithm>
#include <string>

#include "base/macros.h"
#include "base/strings/string_util.h"
#include "build/build_config.h"
#include "components/mime_util/mime_util.h"
#include "net/base/mime_util.h"

namespace {

const base::FilePath::CharType* const kExtraSupportedImageExtensions[] = {
  // RAW picture file types.
  // Some of which are just image/tiff.
  FILE_PATH_LITERAL("3fr"),  // (Hasselblad)
  FILE_PATH_LITERAL("arw"),  // (Sony)
  FILE_PATH_LITERAL("dcr"),  // (Kodak)
  FILE_PATH_LITERAL("dng"),  // (Adobe, Leica, Ricoh, Samsung)
  FILE_PATH_LITERAL("erf"),  // (Epson)
  FILE_PATH_LITERAL("k25"),  // (Kodak)
  FILE_PATH_LITERAL("kdc"),  // (Kodak)
  FILE_PATH_LITERAL("mef"),  // (Mamiya)
  FILE_PATH_LITERAL("mos"),  // (Leaf)
  FILE_PATH_LITERAL("nef"),  // (Nikon)
  FILE_PATH_LITERAL("pef"),  // (Pentax)
  FILE_PATH_LITERAL("sr2"),  // (Sony)
  FILE_PATH_LITERAL("srf"),  // (Sony)

  // More RAW picture file types.
  FILE_PATH_LITERAL("cr2"),  // (Canon - image/x-canon-cr2)
  // Note, some .crw files are just TIFFs.
  FILE_PATH_LITERAL("crw"),  // (Canon - image/x-canon-crw)
  FILE_PATH_LITERAL("mrw"),  // (Minolta - image/x-minolta-mrw)
  FILE_PATH_LITERAL("orf"),  // (Olympus - image/x-olympus-orf)
  FILE_PATH_LITERAL("raf"),  // (Fuji)
  FILE_PATH_LITERAL("rw2"),  // (Panasonic - image/x-panasonic-raw)
  FILE_PATH_LITERAL("x3f"),  // (Sigma - image/x-x3f)

  // There exists many file formats all with the .raw extension. For now, only
  // the following types are supported:
  // - TIFF files with .raw extension - image/tiff
  // - Leica / Panasonic RAW files - image/x-panasonic-raw
  // - Phase One RAW files - image/x-phaseone-raw
  FILE_PATH_LITERAL("raw"),
};

const base::FilePath::CharType* const kExtraSupportedVideoExtensions[] = {
  FILE_PATH_LITERAL("3gp"),
  FILE_PATH_LITERAL("3gpp"),
  FILE_PATH_LITERAL("avi"),
  FILE_PATH_LITERAL("flv"),
  FILE_PATH_LITERAL("mkv"),
  FILE_PATH_LITERAL("mov"),
  FILE_PATH_LITERAL("mpeg"),
  FILE_PATH_LITERAL("mpeg4"),
  FILE_PATH_LITERAL("mpegps"),
  FILE_PATH_LITERAL("mpg"),
  FILE_PATH_LITERAL("wmv"),
};

const base::FilePath::CharType* const kExtraSupportedAudioExtensions[] = {
  // Many of these file types are audio files in the same containers that the
  // MIME sniffer already detects as video/subtype.
  FILE_PATH_LITERAL("aac"),   // audio/mpeg
  FILE_PATH_LITERAL("alac"),  // video/mp4
  FILE_PATH_LITERAL("flac"),  // audio/x-flac
  FILE_PATH_LITERAL("m4b"),   // video/mp4
  FILE_PATH_LITERAL("m4p"),   // video/mp4
  FILE_PATH_LITERAL("wma"),   // video/x-ms-asf
};

bool IsUnsupportedExtension(const base::FilePath::StringType& extension) {
  std::string mime_type;
  return !net::GetMimeTypeFromExtension(extension, &mime_type) ||
         !mime_util::IsSupportedMimeType(mime_type);
}

std::vector<base::FilePath::StringType> GetMediaExtensionList(
    const std::string& mime_type) {
  std::vector<base::FilePath::StringType> extensions;
  net::GetExtensionsForMimeType(mime_type, &extensions);
  std::vector<base::FilePath::StringType>::iterator new_end =
      std::remove_if(extensions.begin(),
                     extensions.end(),
                     &IsUnsupportedExtension);
  extensions.erase(new_end, extensions.end());
  return extensions;
}

}  // namespace

// static
bool MediaPathFilter::ShouldSkip(const base::FilePath& path) {
  const base::FilePath::StringType base_name = path.BaseName().value();
  if (base_name.empty())
    return false;

  // Dot files (aka hidden files)
  if (base_name[0] == '.')
    return true;

  // Mac OS X file.
  if (base_name == FILE_PATH_LITERAL("__MACOSX"))
    return true;

#if defined(OS_WIN)
  DWORD file_attributes = ::GetFileAttributes(path.value().c_str());
  if ((file_attributes != INVALID_FILE_ATTRIBUTES) &&
      ((file_attributes & FILE_ATTRIBUTE_HIDDEN) != 0))
    return true;
#else
  // Windows always creates a recycle bin folder in the attached device to store
  // all the deleted contents. On non-windows operating systems, there is no way
  // to get the hidden attribute of windows recycle bin folders that are present
  // on the attached device. Therefore, compare the file path name to the
  // recycle bin name and exclude those folders. For more details, please refer
  // to http://support.microsoft.com/kb/171694.
  const char win_98_recycle_bin_name[] = "RECYCLED";
  const char win_xp_recycle_bin_name[] = "RECYCLER";
  const char win_vista_recycle_bin_name[] = "$Recycle.bin";
  if (base::StartsWith(base_name, win_98_recycle_bin_name,
                       base::CompareCase::INSENSITIVE_ASCII) ||
      base::StartsWith(base_name, win_xp_recycle_bin_name,
                       base::CompareCase::INSENSITIVE_ASCII) ||
      base::StartsWith(base_name, win_vista_recycle_bin_name,
                       base::CompareCase::INSENSITIVE_ASCII))
    return true;
#endif  // defined(OS_WIN)
  return false;
}

MediaPathFilter::MediaPathFilter()
    : initialized_(false) {
  sequence_checker_.DetachFromSequence();
}

MediaPathFilter::~MediaPathFilter() {
}

bool MediaPathFilter::Match(const base::FilePath& path) {
  return GetType(path) != MEDIA_GALLERY_FILE_TYPE_UNKNOWN;
}

MediaGalleryFileType MediaPathFilter::GetType(const base::FilePath& path) {
  EnsureInitialized();
  MediaFileExtensionMap::const_iterator it =
      media_file_extensions_map_.find(base::ToLowerASCII(path.Extension()));
  if (it == media_file_extensions_map_.end())
    return MEDIA_GALLERY_FILE_TYPE_UNKNOWN;
  return static_cast<MediaGalleryFileType>(it->second);
}

void MediaPathFilter::EnsureInitialized() {
  DCHECK(sequence_checker_.CalledOnValidSequence());
  if (initialized_)
    return;

  // This may require I/O when it calls net::GetExtensionsForMimeType(), so
  // doing this in the ctor and removing |initialized_| would result in a
  // ThreadRestrictions failure.
  AddExtensionsToMediaFileExtensionMap(GetMediaExtensionList("image/*"),
                                       MEDIA_GALLERY_FILE_TYPE_IMAGE);
  AddExtensionsToMediaFileExtensionMap(GetMediaExtensionList("audio/*"),
                                       MEDIA_GALLERY_FILE_TYPE_AUDIO);
  AddExtensionsToMediaFileExtensionMap(GetMediaExtensionList("video/*"),
                                       MEDIA_GALLERY_FILE_TYPE_VIDEO);
  AddAdditionalExtensionsToMediaFileExtensionMap(
      kExtraSupportedImageExtensions, arraysize(kExtraSupportedImageExtensions),
      MEDIA_GALLERY_FILE_TYPE_IMAGE);
  AddAdditionalExtensionsToMediaFileExtensionMap(
      kExtraSupportedAudioExtensions, arraysize(kExtraSupportedAudioExtensions),
      MEDIA_GALLERY_FILE_TYPE_AUDIO);
  AddAdditionalExtensionsToMediaFileExtensionMap(
      kExtraSupportedVideoExtensions, arraysize(kExtraSupportedVideoExtensions),
      MEDIA_GALLERY_FILE_TYPE_VIDEO);

  initialized_ = true;
}

void MediaPathFilter::AddExtensionsToMediaFileExtensionMap(
    const MediaFileExtensionList& extensions_list,
    MediaGalleryFileType type) {
  for (size_t i = 0; i < extensions_list.size(); ++i)
    AddExtensionToMediaFileExtensionMap(extensions_list[i].c_str(), type);
}

void MediaPathFilter::AddAdditionalExtensionsToMediaFileExtensionMap(
    const base::FilePath::CharType* const* extensions_list,
    size_t extensions_list_size,
    MediaGalleryFileType type) {
  for (size_t i = 0; i < extensions_list_size; ++i)
    AddExtensionToMediaFileExtensionMap(extensions_list[i], type);
}

void MediaPathFilter::AddExtensionToMediaFileExtensionMap(
    const base::FilePath::CharType* extension,
    MediaGalleryFileType type) {
  base::FilePath::StringType extension_with_sep =
      base::FilePath::kExtensionSeparator +
      base::FilePath::StringType(extension);
  media_file_extensions_map_[extension_with_sep] |= type;
}
